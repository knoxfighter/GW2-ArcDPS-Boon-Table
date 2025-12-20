#define IMGUI_DEFINE_MATH_OPERATORS
#include "BigTable.h"

#include "../extension/ImGui_Math.h"

namespace ImGuiEx::BigTable {
    // storage
    ImGuiTable* CurrentTable;
	ImPool<ImGuiTable>                Tables;
	ImVector<ImGuiPtrOrIndex>         CurrentTableStack;
	ImVector<float>                   TablesLastTimeActive;        // Last used timestamp of each tables (SOA, for efficient GC)
	ImVector<ImDrawChannel>           DrawChannelsTempMergeBuffer;
	ImChunkStream<ImGuiTableSettings> SettingsTables;    // ImGuiTable .ini settings entries
	
    // Configuration
    static const int TABLE_DRAW_CHANNEL_BG0 = 0;
    static const int TABLE_DRAW_CHANNEL_BG2_FROZEN = 1;
    static const int TABLE_DRAW_CHANNEL_NOCLIP = 2;                     // When using ImGuiTableFlags_NoClip (this becomes the last visible channel)
    static const float TABLE_BORDER_SIZE = 1.0f;                        // FIXME-TABLE: Currently hard-coded because of clipping assumptions with outer borders rendering.
    static const float TABLE_RESIZE_SEPARATOR_HALF_THICKNESS = 4.0f;    // Extend outside inner borders.
    static const float TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER = 0.06f;   // Delay/timer before making the hover feedback (color+cursor) visible because tables/columns tends to be more cramped.

    inline ImGuiTableFlags TableFixFlags(ImGuiTableFlags flags, ImGuiWindow* outer_window)
    {
        // Adjust flags: set default sizing policy
        if ((flags & ImGuiTableFlags_SizingMask_) == 0)
            flags |= ((flags & ImGuiTableFlags_ScrollX) || (outer_window->Flags & ImGuiWindowFlags_AlwaysAutoResize)) ? ImGuiTableFlags_SizingFixedFit : ImGuiTableFlags_SizingStretchSame;

        // Adjust flags: enable NoKeepColumnsVisible when using ImGuiTableFlags_SizingFixedSame
        if ((flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedSame)
            flags |= ImGuiTableFlags_NoKeepColumnsVisible;

        // Adjust flags: enforce borders when resizable
        if (flags & ImGuiTableFlags_Resizable)
            flags |= ImGuiTableFlags_BordersInnerV;

        // Adjust flags: disable NoHostExtendX/NoHostExtendY if we have any scrolling going on
        if (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
            flags &= ~(ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_NoHostExtendY);

        // Adjust flags: NoBordersInBodyUntilResize takes priority over NoBordersInBody
        if (flags & ImGuiTableFlags_NoBordersInBodyUntilResize)
            flags &= ~ImGuiTableFlags_NoBordersInBody;

        // Adjust flags: disable saved settings if there's nothing to save
        if ((flags & (ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable)) == 0)
            flags |= ImGuiTableFlags_NoSavedSettings;

        // Inherit _NoSavedSettings from top-level window (child windows always have _NoSavedSettings set)
#ifdef IMGUI_HAS_DOCK
        ImGuiWindow* window_for_settings = outer_window->RootWindowDockStop;
#else
        ImGuiWindow* window_for_settings = outer_window->RootWindow;
#endif
        if (window_for_settings->Flags & ImGuiWindowFlags_NoSavedSettings)
            flags |= ImGuiTableFlags_NoSavedSettings;

        return flags;
    }

    static inline ImGuiSortDirection TableGetColumnAvailSortDirection(ImGuiTableColumn* column, int n)
    {
        IM_ASSERT(n < column->SortDirectionsAvailCount);
        return (column->SortDirectionsAvailList >> (n << 1)) & 0x03;
    }

    // Fix sort direction if currently set on a value which is unavailable (e.g. activating NoSortAscending/NoSortDescending)
    void TableFixColumnSortDirection(ImGuiTable* table, ImGuiTableColumn* column)
    {
        if (column->SortOrder == -1 || (column->SortDirectionsAvailMask & (1 << column->SortDirection)) != 0)
            return;
        column->SortDirection = (ImU8)TableGetColumnAvailSortDirection(column, 0);
        table->IsSortSpecsDirty = true;
    }

    // Adjust flags: default width mode + stretch columns are not allowed when auto extending
    static void TableSetupColumnFlags(ImGuiTable* table, ImGuiTableColumn* column, ImGuiTableColumnFlags flags_in)
    {
        ImGuiTableColumnFlags flags = flags_in;

        // Sizing Policy
        if ((flags & ImGuiTableColumnFlags_WidthMask_) == 0)
        {
            const ImGuiTableFlags table_sizing_policy = (table->Flags & ImGuiTableFlags_SizingMask_);
            if (table_sizing_policy == ImGuiTableFlags_SizingFixedFit || table_sizing_policy == ImGuiTableFlags_SizingFixedSame)
                flags |= ImGuiTableColumnFlags_WidthFixed;
            else
                flags |= ImGuiTableColumnFlags_WidthStretch;
        }
        else
        {
            IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_WidthMask_)); // Check that only 1 of each set is used.
        }

        // Resize
        if ((table->Flags & ImGuiTableFlags_Resizable) == 0)
            flags |= ImGuiTableColumnFlags_NoResize;

        // Sorting
        if ((flags & ImGuiTableColumnFlags_NoSortAscending) && (flags & ImGuiTableColumnFlags_NoSortDescending))
            flags |= ImGuiTableColumnFlags_NoSort;

        // Indentation
        if ((flags & ImGuiTableColumnFlags_IndentMask_) == 0)
            flags |= (table->Columns.index_from_ptr(column) == 0) ? ImGuiTableColumnFlags_IndentEnable : ImGuiTableColumnFlags_IndentDisable;

        // Alignment
        //if ((flags & ImGuiTableColumnFlags_AlignMask_) == 0)
        //    flags |= ImGuiTableColumnFlags_AlignCenter;
        //IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_AlignMask_)); // Check that only 1 of each set is used.

        // Preserve status flags
        column->Flags = flags | (column->Flags & ImGuiTableColumnFlags_StatusMask_);

        // Build an ordered list of available sort directions
        column->SortDirectionsAvailCount = column->SortDirectionsAvailMask = column->SortDirectionsAvailList = 0;
        if (table->Flags & ImGuiTableFlags_Sortable)
        {
            int count = 0, mask = 0, list = 0;
            if ((flags & ImGuiTableColumnFlags_PreferSortAscending) != 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) { mask |= 1 << ImGuiSortDirection_Ascending;  list |= ImGuiSortDirection_Ascending << (count << 1); count++; }
            if ((flags & ImGuiTableColumnFlags_PreferSortDescending) != 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) { mask |= 1 << ImGuiSortDirection_Descending; list |= ImGuiSortDirection_Descending << (count << 1); count++; }
            if ((flags & ImGuiTableColumnFlags_PreferSortAscending) == 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) { mask |= 1 << ImGuiSortDirection_Ascending;  list |= ImGuiSortDirection_Ascending << (count << 1); count++; }
            if ((flags & ImGuiTableColumnFlags_PreferSortDescending) == 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) { mask |= 1 << ImGuiSortDirection_Descending; list |= ImGuiSortDirection_Descending << (count << 1); count++; }
            if ((table->Flags & ImGuiTableFlags_SortTristate) || count == 0) { mask |= 1 << ImGuiSortDirection_None; count++; }
            column->SortDirectionsAvailList = (ImU8)list;
            column->SortDirectionsAvailMask = (ImU8)mask;
            column->SortDirectionsAvailCount = (ImU8)count;
            TableFixColumnSortDirection(table, column);
        }
    }

    // For reference, the average total _allocation count_ for a table is:
    // + 0 (for ImGuiTable instance, we are pooling allocations in g.Tables)
    // + 1 (for table->RawData allocated below)
    // + 1 (for table->ColumnsNames, if names are used)
    // + 1 (for table->Splitter._Channels)
    // + 2 * active_channels_count (for ImDrawCmd and ImDrawIdx buffers inside channels)
    // Where active_channels_count is variable but often == columns_count or columns_count + 1, see TableSetupDrawChannels() for details.
    // Unused channels don't perform their +2 allocations.
    void TableBeginInitMemory(ImGuiTable* table, int columns_count)
    {
        // Allocate single buffer for our arrays
        ImSpanAllocator<3> span_allocator;
        span_allocator.ReserveBytes(0, columns_count * sizeof(ImGuiTableColumn));
        span_allocator.ReserveBytes(1, columns_count * sizeof(ImGuiTableColumnIdx));
        span_allocator.ReserveBytes(2, columns_count * sizeof(ImGuiTableCellData));
        table->RawData = IM_ALLOC(span_allocator.GetArenaSizeInBytes());
        memset(table->RawData, 0, span_allocator.GetArenaSizeInBytes());
        span_allocator.SetArenaBasePtr(table->RawData);
        span_allocator.GetSpan(0, &table->Columns);
        span_allocator.GetSpan(1, &table->DisplayOrderToIndex);
        span_allocator.GetSpan(2, &table->RowCellData);
    }

    void TableLoadSettings(ImGuiTable* table)
    {
        table->IsSettingsRequestLoad = false;
        if (table->Flags & ImGuiTableFlags_NoSavedSettings)
            return;

        // Bind settings
        ImGuiTableSettings* settings;
        if (table->SettingsOffset == -1)
        {
            settings = TableSettingsFindByID(table->ID);
            if (settings == NULL)
                return;
            if (settings->ColumnsCount != table->ColumnsCount) // Allow settings if columns count changed. We could otherwise decide to return...
                table->IsSettingsDirty = true;
            table->SettingsOffset = SettingsTables.offset_from_ptr(settings);
        }
        else
        {
            settings = TableGetBoundSettings(table);
        }

        table->SettingsLoadedFlags = settings->SaveFlags;
        table->RefScale = settings->RefScale;

        // Serialize ImGuiTableSettings/ImGuiTableColumnSettings into ImGuiTable/ImGuiTableColumn
        ImGuiTableColumnSettings* column_settings = settings->GetColumnSettings();
        ImGuiTableColumnMask display_order_mask;
        for (int data_n = 0; data_n < settings->ColumnsCount; data_n++, column_settings++)
        {
            int column_n = column_settings->Index;
            if (column_n < 0 || column_n >= table->ColumnsCount)
                continue;

            ImGuiTableColumn* column = &table->Columns[column_n];
            if (settings->SaveFlags & ImGuiTableFlags_Resizable)
            {
                if (column_settings->IsStretch)
                    column->StretchWeight = column_settings->WidthOrWeight;
                else
                    column->WidthRequest = column_settings->WidthOrWeight;
                column->AutoFitQueue = 0x00;
            }
            if (settings->SaveFlags & ImGuiTableFlags_Reorderable)
                column->DisplayOrder = column_settings->DisplayOrder;
            else
                column->DisplayOrder = (ImGuiTableColumnIdx)column_n;
            display_order_mask |= ImGuiTableColumnMask(1) << column->DisplayOrder;
            column->IsEnabled = column->IsEnabledNextFrame = column_settings->IsEnabled;
            column->SortOrder = column_settings->SortOrder;
            column->SortDirection = column_settings->SortDirection;
        }

        // Validate and fix invalid display order data
        const ImGuiTableColumnMask expected_display_order_mask = (settings->ColumnsCount == 128) ? ImGuiTableColumnMask(~0, ~0) : (ImGuiTableColumnMask(1) << settings->ColumnsCount) - 1;
        if (display_order_mask != expected_display_order_mask)
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                table->Columns[column_n].DisplayOrder = (ImGuiTableColumnIdx)column_n;

        // Rebuild index
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            table->DisplayOrderToIndex[table->Columns[column_n].DisplayOrder] = (ImGuiTableColumnIdx)column_n;
    }

    // Apply queued resizing/reordering/hiding requests
    void TableBeginApplyRequests(ImGuiTable* table)
    {
        // Handle resizing request
        // (We process this at the first TableBegin of the frame)
        // FIXME-TABLE: Contains columns if our work area doesn't allow for scrolling?
        if (table->InstanceCurrent == 0)
        {
            if (table->ResizedColumn != -1 && table->ResizedColumnNextWidth != FLT_MAX)
                TableSetColumnWidth(table->ResizedColumn, table->ResizedColumnNextWidth);
            table->LastResizedColumn = table->ResizedColumn;
            table->ResizedColumnNextWidth = FLT_MAX;
            table->ResizedColumn = -1;

            // Process auto-fit for single column, which is a special case for stretch columns and fixed columns with FixedSame policy.
            // FIXME-TABLE: Would be nice to redistribute available stretch space accordingly to other weights, instead of giving it all to siblings.
            if (table->AutoFitSingleColumn != -1)
            {
                TableSetColumnWidth(table->AutoFitSingleColumn, table->Columns[table->AutoFitSingleColumn].WidthAuto);
                table->AutoFitSingleColumn = -1;
            }
        }

        // Handle reordering request
        // Note: we don't clear ReorderColumn after handling the request.
        if (table->InstanceCurrent == 0)
        {
            if (table->HeldHeaderColumn == -1 && table->ReorderColumn != -1)
                table->ReorderColumn = -1;
            table->HeldHeaderColumn = -1;
            if (table->ReorderColumn != -1 && table->ReorderColumnDir != 0)
            {
                // We need to handle reordering across hidden columns.
                // In the configuration below, moving C to the right of E will lead to:
                //    ... C [D] E  --->  ... [D] E  C   (Column name/index)
                //    ... 2  3  4        ...  2  3  4   (Display order)
                const int reorder_dir = table->ReorderColumnDir;
                IM_ASSERT(reorder_dir == -1 || reorder_dir == +1);
                IM_ASSERT(table->Flags & ImGuiTableFlags_Reorderable);
                ImGuiTableColumn* src_column = &table->Columns[table->ReorderColumn];
                ImGuiTableColumn* dst_column = &table->Columns[(reorder_dir == -1) ? src_column->PrevEnabledColumn : src_column->NextEnabledColumn];
                IM_UNUSED(dst_column);
                const int src_order = src_column->DisplayOrder;
                const int dst_order = dst_column->DisplayOrder;
                src_column->DisplayOrder = (ImGuiTableColumnIdx)dst_order;
                for (int order_n = src_order + reorder_dir; order_n != dst_order + reorder_dir; order_n += reorder_dir)
                    table->Columns[table->DisplayOrderToIndex[order_n]].DisplayOrder -= (ImGuiTableColumnIdx)reorder_dir;
                IM_ASSERT(dst_column->DisplayOrder == dst_order - reorder_dir);

                // Display order is stored in both columns->IndexDisplayOrder and table->DisplayOrder[],
                // rebuild the later from the former.
                for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                    table->DisplayOrderToIndex[table->Columns[column_n].DisplayOrder] = (ImGuiTableColumnIdx)column_n;
                table->ReorderColumnDir = 0;
                table->IsSettingsDirty = true;
            }
        }

        // Handle display order reset request
        if (table->IsResetDisplayOrderRequest)
        {
            for (int n = 0; n < table->ColumnsCount; n++)
                table->DisplayOrderToIndex[n] = table->Columns[n].DisplayOrder = (ImGuiTableColumnIdx)n;
            table->IsResetDisplayOrderRequest = false;
            table->IsSettingsDirty = true;
        }
    }

    bool BeginTableEx(const char* name, ImGuiID id, int columns_count, ImGuiTableFlags flags, ImGuiWindowFlags subWindowFlags, const ImVec2& outer_size, float inner_width)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* outer_window = ImGui::GetCurrentWindow();
        if (outer_window->SkipItems) // Consistent with other tables + beneficial side effect that assert on miscalling EndTable() will be more visible.
            return false;

        // Sanity checks
        IM_ASSERT(columns_count > 0 && columns_count <= IMGUIEX_TABLE_MAX_COLUMNS && "Only 1..128 columns allowed!");
        if (flags & ImGuiTableFlags_ScrollX)
            IM_ASSERT(inner_width >= 0.0f);

        // If an outer size is specified ahead we will be able to early out when not visible. Exact clipping rules may evolve.
        const bool use_child_window = (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) != 0;
        const ImVec2 avail_size = ImGui::GetContentRegionAvail();
        ImVec2 actual_outer_size = ImGui::CalcItemSize(outer_size, ImMax(avail_size.x, 1.0f), use_child_window ? ImMax(avail_size.y, 1.0f) : 0.0f);
        ImRect outer_rect(outer_window->DC.CursorPos, outer_window->DC.CursorPos + actual_outer_size);
        if (use_child_window && ImGui::IsClippedEx(outer_rect, 0, false))
        {
	        ImGui::ItemSize(outer_rect);
            return false;
        }

        // Acquire storage for the table
        ImGuiTable* table = Tables.GetOrAddByKey(id);
        const int instance_no = (table->LastFrameActive != g.FrameCount) ? 0 : table->InstanceCurrent + 1;
        const ImGuiID instance_id = id + instance_no;
        const ImGuiTableFlags table_last_flags = table->Flags;
        if (instance_no > 0)
            IM_ASSERT(table->ColumnsCount == columns_count && "BeginTable(): Cannot change columns count mid-frame while preserving same ID");

        // Fix flags
        table->IsDefaultSizingPolicy = (flags & ImGuiTableFlags_SizingMask_) == 0;
        flags = TableFixFlags(flags, outer_window);

        // Initialize
        table->ID = id;
        table->Flags = flags;
        table->InstanceCurrent = (ImS16)instance_no;
        table->LastFrameActive = g.FrameCount;
        table->OuterWindow = table->InnerWindow = outer_window;
        table->ColumnsCount = columns_count;
        table->IsLayoutLocked = false;
        table->InnerWidth = inner_width;
        table->UserOuterSize = outer_size;

        // When not using a child window, WorkRect.Max will grow as we append contents.
        if (use_child_window)
        {
            // Ensure no vertical scrollbar appears if we only want horizontal one, to make flag consistent
            // (we have no other way to disable vertical scrollbar of a window while keeping the horizontal one showing)
            ImVec2 override_content_size(FLT_MAX, FLT_MAX);
            if ((flags & ImGuiTableFlags_ScrollX) && !(flags & ImGuiTableFlags_ScrollY))
                override_content_size.y = FLT_MIN;

            // Ensure specified width (when not specified, Stretched columns will act as if the width == OuterWidth and
            // never lead to any scrolling). We don't handle inner_width < 0.0f, we could potentially use it to right-align
            // based on the right side of the child window work rect, which would require knowing ahead if we are going to
            // have decoration taking horizontal spaces (typically a vertical scrollbar).
            if ((flags & ImGuiTableFlags_ScrollX) && inner_width > 0.0f)
                override_content_size.x = inner_width;

            if (override_content_size.x != FLT_MAX || override_content_size.y != FLT_MAX)
	            ImGui::SetNextWindowContentSize(ImVec2(override_content_size.x != FLT_MAX ? override_content_size.x : 0.0f, override_content_size.y != FLT_MAX ? override_content_size.y : 0.0f));

            // Reset scroll if we are reactivating it
            if ((table_last_flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) == 0)
	            ImGui::SetNextWindowScroll(ImVec2(0.0f, 0.0f));

            // Create scrolling region (without border and zero window padding)
            ImGuiWindowFlags child_flags = (flags & ImGuiTableFlags_ScrollX) ? ImGuiWindowFlags_HorizontalScrollbar : ImGuiWindowFlags_None;
        	child_flags |= subWindowFlags;
            ImGui::BeginChildEx(name, instance_id, outer_rect.GetSize(), false, child_flags);
            table->InnerWindow = g.CurrentWindow;
            table->WorkRect = table->InnerWindow->WorkRect;
            table->OuterRect = table->InnerWindow->Rect();
            table->InnerRect = table->InnerWindow->InnerRect;
            IM_ASSERT(table->InnerWindow->WindowPadding.x == 0.0f && table->InnerWindow->WindowPadding.y == 0.0f && table->InnerWindow->WindowBorderSize == 0.0f);
        }
        else
        {
            // For non-scrolling tables, WorkRect == OuterRect == InnerRect.
            // But at this point we do NOT have a correct value for .Max.y (unless a height has been explicitly passed in). It will only be updated in EndTable().
            table->WorkRect = table->OuterRect = table->InnerRect = outer_rect;
        }

        // Push a standardized ID for both child-using and not-child-using tables
        ImGui::PushOverrideID(instance_id);

        // Backup a copy of host window members we will modify
        ImGuiWindow* inner_window = table->InnerWindow;
        table->HostIndentX = inner_window->DC.Indent.x;
        table->HostClipRect = inner_window->ClipRect;
        table->HostSkipItems = inner_window->SkipItems;
        table->HostBackupWorkRect = inner_window->WorkRect;
        table->HostBackupParentWorkRect = inner_window->ParentWorkRect;
        table->HostBackupColumnsOffset = outer_window->DC.ColumnsOffset;
        table->HostBackupPrevLineSize = inner_window->DC.PrevLineSize;
        table->HostBackupCurrLineSize = inner_window->DC.CurrLineSize;
        table->HostBackupCursorMaxPos = inner_window->DC.CursorMaxPos;
        table->HostBackupItemWidth = outer_window->DC.ItemWidth;
        table->HostBackupItemWidthStackSize = outer_window->DC.ItemWidthStack.Size;
        inner_window->DC.PrevLineSize = inner_window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);

        // Padding and Spacing
        // - None               ........Content..... Pad .....Content........
        // - PadOuter           | Pad ..Content..... Pad .....Content.. Pad |
        // - PadInner           ........Content.. Pad | Pad ..Content........
        // - PadOuter+PadInner  | Pad ..Content.. Pad | Pad ..Content.. Pad |
        const bool pad_outer_x = (flags & ImGuiTableFlags_NoPadOuterX) ? false : (flags & ImGuiTableFlags_PadOuterX) ? true : (flags & ImGuiTableFlags_BordersOuterV) != 0;
        const bool pad_inner_x = (flags & ImGuiTableFlags_NoPadInnerX) ? false : true;
        const float inner_spacing_for_border = (flags & ImGuiTableFlags_BordersInnerV) ? TABLE_BORDER_SIZE : 0.0f;
        const float inner_spacing_explicit = (pad_inner_x && (flags & ImGuiTableFlags_BordersInnerV) == 0) ? g.Style.CellPadding.x : 0.0f;
        const float inner_padding_explicit = (pad_inner_x && (flags & ImGuiTableFlags_BordersInnerV) != 0) ? g.Style.CellPadding.x : 0.0f;
        table->CellSpacingX1 = inner_spacing_explicit + inner_spacing_for_border;
        table->CellSpacingX2 = inner_spacing_explicit;
        table->CellPaddingX = inner_padding_explicit;
        table->CellPaddingY = g.Style.CellPadding.y;

        const float outer_padding_for_border = (flags & ImGuiTableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
        const float outer_padding_explicit = pad_outer_x ? g.Style.CellPadding.x : 0.0f;
        table->OuterPaddingX = (outer_padding_for_border + outer_padding_explicit) - table->CellPaddingX;

        table->CurrentColumn = -1;
        table->CurrentRow = -1;
        table->RowBgColorCounter = 0;
        table->LastRowFlags = ImGuiTableRowFlags_None;
        table->InnerClipRect = (inner_window == outer_window) ? table->WorkRect : inner_window->ClipRect;
        table->InnerClipRect.ClipWith(table->WorkRect);     // We need this to honor inner_width
        table->InnerClipRect.ClipWithFull(table->HostClipRect);
        table->InnerClipRect.Max.y = (flags & ImGuiTableFlags_NoHostExtendY) ? ImMin(table->InnerClipRect.Max.y, inner_window->WorkRect.Max.y) : inner_window->ClipRect.Max.y;

        table->RowPosY1 = table->RowPosY2 = table->WorkRect.Min.y; // This is needed somehow
        table->RowTextBaseline = 0.0f; // This will be cleared again by TableBeginRow()
        table->FreezeRowsRequest = table->FreezeRowsCount = 0; // This will be setup by TableSetupScrollFreeze(), if any
        table->FreezeColumnsRequest = table->FreezeColumnsCount = 0;
        table->IsUnfrozenRows = true;
        table->DeclColumnsCount = 0;

        // Using opaque colors facilitate overlapping elements of the grid
        table->BorderColorStrong = ImGui::GetColorU32(ImGuiCol_TableBorderStrong);
        table->BorderColorLight = ImGui::GetColorU32(ImGuiCol_TableBorderLight);

        // Make table current
        const int table_idx = Tables.GetIndex(table);
        CurrentTableStack.push_back(ImGuiPtrOrIndex(table_idx));
        CurrentTable = table;
        outer_window->DC.CurrentTableIdx = table_idx;
        if (inner_window != outer_window) // So EndChild() within the inner window can restore the table properly.
            inner_window->DC.CurrentTableIdx = table_idx;

        if ((table_last_flags & ImGuiTableFlags_Reorderable) && (flags & ImGuiTableFlags_Reorderable) == 0)
            table->IsResetDisplayOrderRequest = true;

        // Mark as used
        if (table_idx >= TablesLastTimeActive.Size)
            TablesLastTimeActive.resize(table_idx + 1, -1.0f);
        TablesLastTimeActive[table_idx] = (float)g.Time;
        table->MemoryCompacted = false;

        // Setup memory buffer (clear data if columns count changed)
        const int stored_size = table->Columns.size();
        if (stored_size != 0 && stored_size != columns_count)
        {
            IM_FREE(table->RawData);
            table->RawData = NULL;
        }
        if (table->RawData == NULL)
        {
	        TableBeginInitMemory(table, columns_count);
            table->IsInitializing = table->IsSettingsRequestLoad = true;
        }
        if (table->IsResetAllRequest)
	        TableResetSettings(table);
        if (table->IsInitializing)
        {
            // Initialize
            table->SettingsOffset = -1;
            table->IsSortSpecsDirty = true;
            table->InstanceInteracted = -1;
            table->ContextPopupColumn = -1;
            table->ReorderColumn = table->ResizedColumn = table->LastResizedColumn = -1;
            table->AutoFitSingleColumn = -1;
            table->HoveredColumnBody = table->HoveredColumnBorder = -1;
            for (int n = 0; n < columns_count; n++)
            {
                ImGuiTableColumn* column = &table->Columns[n];
                float width_auto = column->WidthAuto;
                *column = ImGuiTableColumn();
                column->WidthAuto = width_auto;
                column->IsPreserveWidthAuto = true; // Preserve WidthAuto when reinitializing a live table: not technically necessary but remove a visible flicker
                column->DisplayOrder = table->DisplayOrderToIndex[n] = (ImGuiTableColumnIdx)n;
                column->IsEnabled = column->IsEnabledNextFrame = true;
            }
        }

        // Load settings
        if (table->IsSettingsRequestLoad)
            TableLoadSettings(table);

        // Handle DPI/font resize
        // This is designed to facilitate DPI changes with the assumption that e.g. style.CellPadding has been scaled as well.
        // It will also react to changing fonts with mixed results. It doesn't need to be perfect but merely provide a decent transition.
        // FIXME-DPI: Provide consistent standards for reference size. Perhaps using g.CurrentDpiScale would be more self explanatory.
        // This is will lead us to non-rounded WidthRequest in columns, which should work but is a poorly tested path.
        const float new_ref_scale_unit = g.FontSize; // g.Font->GetCharAdvance('A') ?
        if (table->RefScale != 0.0f && table->RefScale != new_ref_scale_unit)
        {
            const float scale_factor = new_ref_scale_unit / table->RefScale;
            //IMGUI_DEBUG_LOG("[table] %08X RefScaleUnit %.3f -> %.3f, scaling width by %.3f\n", table->ID, table->RefScaleUnit, new_ref_scale_unit, scale_factor);
            for (int n = 0; n < columns_count; n++)
                table->Columns[n].WidthRequest = table->Columns[n].WidthRequest * scale_factor;
        }
        table->RefScale = new_ref_scale_unit;

        // Disable output until user calls TableNextRow() or TableNextColumn() leading to the TableUpdateLayout() call..
        // This is not strictly necessary but will reduce cases were "out of table" output will be misleading to the user.
        // Because we cannot safely assert in EndTable() when no rows have been created, this seems like our best option.
        inner_window->SkipItems = true;

        // Clear names
        // At this point the ->NameOffset field of each column will be invalid until TableUpdateLayout() or the first call to TableSetupColumn()
        if (table->ColumnsNames.Buf.Size > 0)
            table->ColumnsNames.Buf.resize(0);

        // Apply queued resizing/reordering/hiding requests
        TableBeginApplyRequests(table);

        return true;
    }

    bool BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags, ImGuiWindowFlags subWindowFlags, const ImVec2& outer_size, float inner_width)
    {
	    ImGuiID id = ImGui::GetID(str_id);
	    return BeginTableEx(str_id, id, columns_count, flags, subWindowFlags, outer_size, inner_width);
    }

    // Restore initial state of table (with or without saved settings)
    void TableResetSettings(ImGuiTable* table)
    {
        table->IsInitializing = table->IsSettingsDirty = true;
        table->IsResetAllRequest = false;
        table->IsSettingsRequestLoad = false;                   // Don't reload from ini
        table->SettingsLoadedFlags = ImGuiTableFlags_None;      // Mark as nothing loaded so our initialized data becomes authoritative
    }

    // Find existing settings
    ImGuiTableSettings* TableSettingsFindByID(ImGuiID id)
    {
        // FIXME-OPT: Might want to store a lookup map for this?
        for (ImGuiTableSettings* settings = SettingsTables.begin(); settings != NULL; settings = SettingsTables.next_chunk(settings))
            if (settings->ID == id)
                return settings;
        return nullptr;
    }

    // Get settings for a given table, NULL if none
    ImGuiTableSettings* TableGetBoundSettings(ImGuiTable* table)
    {
        if (table->SettingsOffset != -1)
        {
            ImGuiTableSettings* settings = SettingsTables.ptr_from_offset(table->SettingsOffset);
            IM_ASSERT(settings->ID == table->ID);
            if (settings->ColumnsCountMax >= table->ColumnsCount)
                return settings; // OK
            settings->ID = 0; // Invalidate storage, we won't fit because of a count change
        }
        return nullptr;
    }

    void TableUpdateColumnsWeightFromWidth(ImGuiTable* table)
    {
        IM_ASSERT(table->LeftMostStretchedColumn != -1 && table->RightMostStretchedColumn != -1);

        // Measure existing quantity
        float visible_weight = 0.0f;
        float visible_width = 0.0f;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (!column->IsEnabled || !(column->Flags & ImGuiTableColumnFlags_WidthStretch))
                continue;
            IM_ASSERT(column->StretchWeight > 0.0f);
            visible_weight += column->StretchWeight;
            visible_width += column->WidthRequest;
        }
        IM_ASSERT(visible_weight > 0.0f && visible_width > 0.0f);

        // Apply new weights
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (!column->IsEnabled || !(column->Flags & ImGuiTableColumnFlags_WidthStretch))
                continue;
            column->StretchWeight = (column->WidthRequest / visible_width) * visible_weight;
            IM_ASSERT(column->StretchWeight > 0.0f);
        }
    }

    // 'width' = inner column width, without padding
    void TableSetColumnWidth(int column_n, float width)
    {
        ImGuiTable* table = CurrentTable;
        IM_ASSERT(table != NULL && table->IsLayoutLocked == false);
        IM_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
        ImGuiTableColumn* column_0 = &table->Columns[column_n];
        float column_0_width = width;

        // Apply constraints early
        // Compare both requested and actual given width to avoid overwriting requested width when column is stuck (minimum size, bounded)
        IM_ASSERT(table->MinColumnWidth > 0.0f);
        const float min_width = table->MinColumnWidth;
        const float max_width = ImMax(min_width, TableGetMaxColumnWidth(table, column_n));
        column_0_width = ImClamp(column_0_width, min_width, max_width);
        if (column_0->WidthGiven == column_0_width || column_0->WidthRequest == column_0_width)
            return;

        //IMGUI_DEBUG_LOG("TableSetColumnWidth(%d, %.1f->%.1f)\n", column_0_idx, column_0->WidthGiven, column_0_width);
        ImGuiTableColumn* column_1 = (column_0->NextEnabledColumn != -1) ? &table->Columns[column_0->NextEnabledColumn] : NULL;

        // In this surprisingly not simple because of how we support mixing Fixed and multiple Stretch columns.
        // - All fixed: easy.
        // - All stretch: easy.
        // - One or more fixed + one stretch: easy.
        // - One or more fixed + more than one stretch: tricky.
        // Qt when manual resize is enabled only support a single _trailing_ stretch column.

        // When forwarding resize from Wn| to Fn+1| we need to be considerate of the _NoResize flag on Fn+1.
        // FIXME-TABLE: Find a way to rewrite all of this so interactions feel more consistent for the user.
        // Scenarios:
        // - F1 F2 F3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. Subsequent columns will be offset.
        // - F1 F2 F3  resize from F3|          --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered.
        // - F1 F2 W3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered, but it doesn't make much sense as the Stretch column will always be minimal size.
        // - F1 F2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
        // - W1 W2 W3  resize from W1| or W2|   --> ok
        // - W1 W2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
        // - W1 F2 F3  resize from F3|          --> ok: no-op (disabled by Resize Rule 1)
        // - W1 F2     resize from F2|          --> ok: no-op (disabled by Resize Rule 1)
        // - W1 W2 F3  resize from W1| or W2|   --> ok
        // - W1 F2 W3  resize from W1| or F2|   --> ok
        // - F1 W2 F3  resize from W2|          --> ok
        // - F1 W3 F2  resize from W3|          --> ok
        // - W1 F2 F3  resize from W1|          --> ok: equivalent to resizing |F2. F3 will not move.
        // - W1 F2 F3  resize from F2|          --> ok
        // All resizes from a Wx columns are locking other columns.

        // Possible improvements:
        // - W1 W2 W3  resize W1|               --> to not be stuck, both W2 and W3 would stretch down. Seems possible to fix. Would be most beneficial to simplify resize of all-weighted columns.
        // - W3 F1 F2  resize W3|               --> to not be stuck past F1|, both F1 and F2 would need to stretch down, which would be lossy or ambiguous. Seems hard to fix.

        // [Resize Rule 1] Can't resize from right of right-most visible column if there is any Stretch column. Implemented in TableUpdateLayout().

        // If we have all Fixed columns OR resizing a Fixed column that doesn't come after a Stretch one, we can do an offsetting resize.
        // This is the preferred resize path
        if (column_0->Flags & ImGuiTableColumnFlags_WidthFixed)
            if (!column_1 || table->LeftMostStretchedColumn == -1 || table->Columns[table->LeftMostStretchedColumn].DisplayOrder >= column_0->DisplayOrder)
            {
                column_0->WidthRequest = column_0_width;
                table->IsSettingsDirty = true;
                return;
            }

        // We can also use previous column if there's no next one (this is used when doing an auto-fit on the right-most stretch column)
        if (column_1 == NULL)
            column_1 = (column_0->PrevEnabledColumn != -1) ? &table->Columns[column_0->PrevEnabledColumn] : NULL;
        if (column_1 == NULL)
            return;

        // Resizing from right-side of a Stretch column before a Fixed column forward sizing to left-side of fixed column.
        // (old_a + old_b == new_a + new_b) --> (new_a == old_a + old_b - new_b)
        float column_1_width = ImMax(column_1->WidthRequest - (column_0_width - column_0->WidthRequest), min_width);
        column_0_width = column_0->WidthRequest + column_1->WidthRequest - column_1_width;
        IM_ASSERT(column_0_width > 0.0f && column_1_width > 0.0f);
        column_0->WidthRequest = column_0_width;
        column_1->WidthRequest = column_1_width;
        if ((column_0->Flags | column_1->Flags) & ImGuiTableColumnFlags_WidthStretch)
            TableUpdateColumnsWeightFromWidth(table);
        table->IsSettingsDirty = true;
    }

    // Maximum column content width given current layout. Use column->MinX so this value on a per-column basis.
    float TableGetMaxColumnWidth(const ImGuiTable* table, int column_n)
    {
        const ImGuiTableColumn* column = &table->Columns[column_n];
        float max_width = FLT_MAX;
        const float min_column_distance = table->MinColumnWidth + table->CellPaddingX * 2.0f + table->CellSpacingX1 + table->CellSpacingX2;
        if (table->Flags & ImGuiTableFlags_ScrollX)
        {
            // Frozen columns can't reach beyond visible width else scrolling will naturally break.
            if (column->DisplayOrder < table->FreezeColumnsRequest)
            {
                max_width = (table->InnerClipRect.Max.x - (table->FreezeColumnsRequest - column->DisplayOrder) * min_column_distance) - column->MinX;
                max_width = max_width - table->OuterPaddingX - table->CellPaddingX - table->CellSpacingX2;
            }
        }
        else if ((table->Flags & ImGuiTableFlags_NoKeepColumnsVisible) == 0)
        {
            // If horizontal scrolling if disabled, we apply a final lossless shrinking of columns in order to make
            // sure they are all visible. Because of this we also know that all of the columns will always fit in
            // table->WorkRect and therefore in table->InnerRect (because ScrollX is off)
            // FIXME-TABLE: This is solved incorrectly but also quite a difficult problem to fix as we also want ClipRect width to match.
            // See "table_width_distrib" and "table_width_keep_visible" tests
            max_width = table->WorkRect.Max.x - (table->ColumnsEnabledCount - column->IndexWithinEnabledSet - 1) * min_column_distance - column->MinX;
            //max_width -= table->CellSpacingX1;
            max_width -= table->CellSpacingX2;
            max_width -= table->CellPaddingX * 2.0f;
            max_width -= table->OuterPaddingX;
        }
        return max_width;
    }

    // See "COLUMN SIZING POLICIES" comments at the top of this file
	// If (init_width_or_weight <= 0.0f) it is ignored
    void TableSetupColumn(const char* label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_id)
    {
        ImGuiTable* table = CurrentTable;
        IM_ASSERT(table != NULL && "Need to call TableSetupColumn() after BeginTable()!");
        IM_ASSERT(table->IsLayoutLocked == false && "Need to call call TableSetupColumn() before first row!");
        IM_ASSERT((flags & ImGuiTableColumnFlags_StatusMask_) == 0 && "Illegal to pass StatusMask values to TableSetupColumn()");
        if (table->DeclColumnsCount >= table->ColumnsCount)
        {
            IM_ASSERT_USER_ERROR(table->DeclColumnsCount < table->ColumnsCount, "Called TableSetupColumn() too many times!");
            return;
        }

        ImGuiTableColumn* column = &table->Columns[table->DeclColumnsCount];
        table->DeclColumnsCount++;

        // Assert when passing a width or weight if policy is entirely left to default, to avoid storing width into weight and vice-versa.
        // Give a grace to users of ImGuiTableFlags_ScrollX.
        if (table->IsDefaultSizingPolicy && (flags & ImGuiTableColumnFlags_WidthMask_) == 0 && (flags & ImGuiTableFlags_ScrollX) == 0)
            IM_ASSERT(init_width_or_weight <= 0.0f && "Can only specify width/weight if sizing policy is set explicitely in either Table or Column.");

        // When passing a width automatically enforce WidthFixed policy
        // (whereas TableSetupColumnFlags would default to WidthAuto if table is not Resizable)
        if ((flags & ImGuiTableColumnFlags_WidthMask_) == 0 && init_width_or_weight > 0.0f)
            if ((table->Flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedFit || (table->Flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedSame)
                flags |= ImGuiTableColumnFlags_WidthFixed;

        TableSetupColumnFlags(table, column, flags);
        column->UserID = user_id;
        flags = column->Flags;

        // Initialize defaults
        column->InitStretchWeightOrWidth = init_width_or_weight;
        if (table->IsInitializing)
        {
            // Init width or weight
            if (column->WidthRequest < 0.0f && column->StretchWeight < 0.0f)
            {
                if ((flags & ImGuiTableColumnFlags_WidthFixed) && init_width_or_weight > 0.0f)
                    column->WidthRequest = init_width_or_weight;
                if (flags & ImGuiTableColumnFlags_WidthStretch)
                    column->StretchWeight = (init_width_or_weight > 0.0f) ? init_width_or_weight : -1.0f;

                // Disable auto-fit if an explicit width/weight has been specified
                if (init_width_or_weight > 0.0f)
                    column->AutoFitQueue = 0x00;
            }

            // Init default visibility/sort state
            if ((flags & ImGuiTableColumnFlags_DefaultHide) && (table->SettingsLoadedFlags & ImGuiTableFlags_Hideable) == 0) {
            	column->IsEnabled = column->IsEnabledNextFrame = false;
            }
            if (flags & ImGuiTableColumnFlags_DefaultSort && (table->SettingsLoadedFlags & ImGuiTableFlags_Sortable) == 0)
            {
                column->SortOrder = 0; // Multiple columns using _DefaultSort will be reassigned unique SortOrder values when building the sort specs.
                column->SortDirection = (column->Flags & ImGuiTableColumnFlags_PreferSortDescending) ? (ImS8)ImGuiSortDirection_Descending : (ImU8)(ImGuiSortDirection_Ascending);
            }
        }

        // Store name (append with zero-terminator in contiguous buffer)
        column->NameOffset = -1;
        if (label != NULL && label[0] != 0)
        {
            column->NameOffset = (ImS16)table->ColumnsNames.size();
            table->ColumnsNames.append(label, label + strlen(label) + 1);
        }
    }

    // [Internal] Called by TableNextRow()/TableSetColumnIndex()/TableNextColumn()
    void TableEndCell(ImGuiTable* table)
    {
        ImGuiTableColumn* column = &table->Columns[table->CurrentColumn];
        ImGuiWindow* window = table->InnerWindow;

        // Report maximum position so we can infer content size per column.
        float* p_max_pos_x;
        if (table->RowFlags & ImGuiTableRowFlags_Headers)
            p_max_pos_x = &column->ContentMaxXHeadersUsed;  // Useful in case user submit contents in header row that is not a TableHeader() call
        else
            p_max_pos_x = table->IsUnfrozenRows ? &column->ContentMaxXUnfrozen : &column->ContentMaxXFrozen;
        *p_max_pos_x = ImMax(*p_max_pos_x, window->DC.CursorMaxPos.x);
        table->RowPosY2 = ImMax(table->RowPosY2, window->DC.CursorMaxPos.y + table->CellPaddingY);
        column->ItemWidth = window->DC.ItemWidth;

        // Propagate text baseline for the entire row
        // FIXME-TABLE: Here we propagate text baseline from the last line of the cell.. instead of the first one.
        table->RowTextBaseline = ImMax(table->RowTextBaseline, window->DC.PrevLineTextBaseOffset);
    }

    // Return the cell rectangle based on currently known height.
	// - Important: we generally don't know our row height until the end of the row, so Max.y will be incorrect in many situations.
	//   The only case where this is correct is if we provided a min_row_height to TableNextRow() and don't go below it.
	// - Important: if ImGuiTableFlags_PadOuterX is set but ImGuiTableFlags_PadInnerX is not set, the outer-most left and right
	//   columns report a small offset so their CellBgRect can extend up to the outer border.
    ImRect TableGetCellBgRect(const ImGuiTable* table, int column_n)
    {
        const ImGuiTableColumn* column = &table->Columns[column_n];
        float x1 = column->MinX;
        float x2 = column->MaxX;
        if (column->PrevEnabledColumn == -1)
            x1 -= table->CellSpacingX1;
        if (column->NextEnabledColumn == -1)
            x2 += table->CellSpacingX2;
        return ImRect(x1, table->RowPosY1, x2, table->RowPosY2);
    }

    // [Internal] Small optimization to avoid calls to PopClipRect/SetCurrentChannel/PushClipRect in sequences,
	// they would meddle many times with the underlying ImDrawCmd.
	// Instead, we do a preemptive overwrite of clipping rectangle _without_ altering the command-buffer and let
	// the subsequent single call to SetCurrentChannel() does it things once.
    void SetWindowClipRectBeforeSetChannel(ImGuiWindow* window, const ImRect& clip_rect)
    {
        ImVec4 clip_rect_vec4 = clip_rect.ToVec4();
        window->ClipRect = clip_rect;
        window->DrawList->_CmdHeader.ClipRect = clip_rect_vec4;
        window->DrawList->_ClipRectStack.Data[window->DrawList->_ClipRectStack.Size - 1] = clip_rect_vec4;
    }
	
    // [Internal] Called by TableNextRow()
    void TableEndRow(ImGuiTable* table)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        IM_ASSERT(window == table->InnerWindow);
        IM_ASSERT(table->IsInsideRow);

        if (table->CurrentColumn != -1)
            TableEndCell(table);

        // Position cursor at the bottom of our row so it can be used for e.g. clipping calculation. However it is
        // likely that the next call to TableBeginCell() will reposition the cursor to take account of vertical padding.
        window->DC.CursorPos.y = table->RowPosY2;

        // Row background fill
        const float bg_y1 = table->RowPosY1;
        const float bg_y2 = table->RowPosY2;
        const bool unfreeze_rows_actual = (table->CurrentRow + 1 == table->FreezeRowsCount);
        const bool unfreeze_rows_request = (table->CurrentRow + 1 == table->FreezeRowsRequest);
        if (table->CurrentRow == 0)
            table->LastFirstRowHeight = bg_y2 - bg_y1;

        const bool is_visible = (bg_y2 >= table->InnerClipRect.Min.y && bg_y1 <= table->InnerClipRect.Max.y);
        if (is_visible)
        {
            // Decide of background color for the row
            ImU32 bg_col0 = 0;
            ImU32 bg_col1 = 0;
            if (table->RowBgColor[0] != IM_COL32_DISABLE)
                bg_col0 = table->RowBgColor[0];
            else if (table->Flags & ImGuiTableFlags_RowBg)
                bg_col0 = ImGui::GetColorU32((table->RowBgColorCounter & 1) ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg);
            if (table->RowBgColor[1] != IM_COL32_DISABLE)
                bg_col1 = table->RowBgColor[1];

            // Decide of top border color
            ImU32 border_col = 0;
            const float border_size = TABLE_BORDER_SIZE;
            if (table->CurrentRow > 0 || table->InnerWindow == table->OuterWindow)
                if (table->Flags & ImGuiTableFlags_BordersInnerH)
                    border_col = (table->LastRowFlags & ImGuiTableRowFlags_Headers) ? table->BorderColorStrong : table->BorderColorLight;

            const bool draw_cell_bg_color = table->RowCellDataCurrent >= 0;
            const bool draw_strong_bottom_border = unfreeze_rows_actual;
            if ((bg_col0 | bg_col1 | border_col) != 0 || draw_strong_bottom_border || draw_cell_bg_color)
            {
                // In theory we could call SetWindowClipRectBeforeSetChannel() but since we know TableEndRow() is
                // always followed by a change of clipping rectangle we perform the smallest overwrite possible here.
                if ((table->Flags & ImGuiTableFlags_NoClip) == 0)
                    window->DrawList->_CmdHeader.ClipRect = table->Bg0ClipRectForDrawCmd.ToVec4();
                table->DrawSplitter.SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_BG0);
            }

            // Draw row background
            // We soft/cpu clip this so all backgrounds and borders can share the same clipping rectangle
            if (bg_col0 || bg_col1)
            {
                ImRect row_rect(table->WorkRect.Min.x, bg_y1, table->WorkRect.Max.x, bg_y2);
                row_rect.ClipWith(table->BgClipRect);
                if (bg_col0 != 0 && row_rect.Min.y < row_rect.Max.y)
                    window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col0);
                if (bg_col1 != 0 && row_rect.Min.y < row_rect.Max.y)
                    window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col1);
            }

            // Draw cell background color
            if (draw_cell_bg_color)
            {
                ImGuiTableCellData* cell_data_end = &table->RowCellData[table->RowCellDataCurrent];
                for (ImGuiTableCellData* cell_data = &table->RowCellData[0]; cell_data <= cell_data_end; cell_data++)
                {
                    const ImGuiTableColumn* column = &table->Columns[cell_data->Column];
                    ImRect cell_bg_rect = TableGetCellBgRect(table, cell_data->Column);
                    cell_bg_rect.ClipWith(table->BgClipRect);
                    cell_bg_rect.Min.x = ImMax(cell_bg_rect.Min.x, column->ClipRect.Min.x);     // So that first column after frozen one gets clipped
                    cell_bg_rect.Max.x = ImMin(cell_bg_rect.Max.x, column->MaxX);
                    window->DrawList->AddRectFilled(cell_bg_rect.Min, cell_bg_rect.Max, cell_data->BgColor);
                }
            }

            // Draw top border
            if (border_col && bg_y1 >= table->BgClipRect.Min.y && bg_y1 < table->BgClipRect.Max.y)
                window->DrawList->AddLine(ImVec2(table->BorderX1, bg_y1), ImVec2(table->BorderX2, bg_y1), border_col, border_size);

            // Draw bottom border at the row unfreezing mark (always strong)
            if (draw_strong_bottom_border && bg_y2 >= table->BgClipRect.Min.y && bg_y2 < table->BgClipRect.Max.y)
                window->DrawList->AddLine(ImVec2(table->BorderX1, bg_y2), ImVec2(table->BorderX2, bg_y2), table->BorderColorStrong, border_size);
        }

        // End frozen rows (when we are past the last frozen row line, teleport cursor and alter clipping rectangle)
        // We need to do that in TableEndRow() instead of TableBeginRow() so the list clipper can mark end of row and
        // get the new cursor position.
        if (unfreeze_rows_request)
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            {
                ImGuiTableColumn* column = &table->Columns[column_n];
                column->NavLayerCurrent = (ImS8)((column_n < table->FreezeColumnsCount) ? ImGuiNavLayer_Menu : ImGuiNavLayer_Main);
            }
        if (unfreeze_rows_actual)
        {
            IM_ASSERT(table->IsUnfrozenRows == false);
            table->IsUnfrozenRows = true;

            // BgClipRect starts as table->InnerClipRect, reduce it now and make BgClipRectForDrawCmd == BgClipRect
            float y0 = ImMax(table->RowPosY2 + 1, window->InnerClipRect.Min.y);
            table->BgClipRect.Min.y = table->Bg2ClipRectForDrawCmd.Min.y = ImMin(y0, window->InnerClipRect.Max.y);
            table->BgClipRect.Max.y = table->Bg2ClipRectForDrawCmd.Max.y = window->InnerClipRect.Max.y;
            table->Bg2DrawChannelCurrent = table->Bg2DrawChannelUnfrozen;
            IM_ASSERT(table->Bg2ClipRectForDrawCmd.Min.y <= table->Bg2ClipRectForDrawCmd.Max.y);

            float row_height = table->RowPosY2 - table->RowPosY1;
            table->RowPosY2 = window->DC.CursorPos.y = table->WorkRect.Min.y + table->RowPosY2 - table->OuterRect.Min.y;
            table->RowPosY1 = table->RowPosY2 - row_height;
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            {
                ImGuiTableColumn* column = &table->Columns[column_n];
                column->DrawChannelCurrent = column->DrawChannelUnfrozen;
                column->ClipRect.Min.y = table->Bg2ClipRectForDrawCmd.Min.y;
            }

            // Update cliprect ahead of TableBeginCell() so clipper can access to new ClipRect->Min.y
            SetWindowClipRectBeforeSetChannel(window, table->Columns[0].ClipRect);
            table->DrawSplitter.SetCurrentChannel(window->DrawList, table->Columns[0].DrawChannelCurrent);
        }

        if (!(table->RowFlags & ImGuiTableRowFlags_Headers))
            table->RowBgColorCounter++;
        table->IsInsideRow = false;
    }

    void TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n)
    {
        ImGuiTable* table = CurrentTable;
        IM_ASSERT(target != ImGuiTableBgTarget_None);

        if (color == IM_COL32_DISABLE)
            color = 0;

        // We cannot draw neither the cell or row background immediately as we don't know the row height at this point in time.
        switch (target)
        {
        case ImGuiTableBgTarget_CellBg:
        {
            if (table->RowPosY1 > table->InnerClipRect.Max.y) // Discard
                return;
            if (column_n == -1)
                column_n = table->CurrentColumn;
            if ((table->VisibleMaskByIndex & (ImGuiTableColumnMask(1) << column_n)) == 0)
                return;
            if (table->RowCellDataCurrent < 0 || table->RowCellData[table->RowCellDataCurrent].Column != column_n)
                table->RowCellDataCurrent++;
            ImGuiTableCellData* cell_data = &table->RowCellData[table->RowCellDataCurrent];
            cell_data->BgColor = color;
            cell_data->Column = (ImGuiTableColumnIdx)column_n;
            break;
        }
        case ImGuiTableBgTarget_RowBg0:
        case ImGuiTableBgTarget_RowBg1:
        {
            if (table->RowPosY1 > table->InnerClipRect.Max.y) // Discard
                return;
            IM_ASSERT(column_n == -1);
            int bg_idx = (target == ImGuiTableBgTarget_RowBg1) ? 1 : 0;
            table->RowBgColor[bg_idx] = color;
            break;
        }
        default:
            IM_ASSERT(0);
        }
    }

    // [Internal] Called by TableNextRow()
    void TableBeginRow(ImGuiTable* table)
    {
        ImGuiWindow* window = table->InnerWindow;
        IM_ASSERT(!table->IsInsideRow);

        // New row
        table->CurrentRow++;
        table->CurrentColumn = -1;
        table->RowBgColor[0] = table->RowBgColor[1] = IM_COL32_DISABLE;
        table->RowCellDataCurrent = -1;
        table->IsInsideRow = true;

        // Begin frozen rows
        float next_y1 = table->RowPosY2;
        if (table->CurrentRow == 0 && table->FreezeRowsCount > 0)
            next_y1 = window->DC.CursorPos.y = table->OuterRect.Min.y;

        table->RowPosY1 = table->RowPosY2 = next_y1;
        table->RowTextBaseline = 0.0f;
        table->RowIndentOffsetX = window->DC.Indent.x - table->HostIndentX; // Lock indent
        window->DC.PrevLineTextBaseOffset = 0.0f;
        window->DC.CursorMaxPos.y = next_y1;

        // Making the header BG color non-transparent will allow us to overlay it multiple times when handling smooth dragging.
        if (table->RowFlags & ImGuiTableRowFlags_Headers)
        {
            TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
            if (table->CurrentRow == 0)
                table->IsUsingHeaders = true;
        }
    }

    void TableSortSpecsSanitize(ImGuiTable* table)
    {
        IM_ASSERT(table->Flags & ImGuiTableFlags_Sortable);

        // Clear SortOrder from hidden column and verify that there's no gap or duplicate.
        int sort_order_count = 0;
        ImGuiTableColumnMask sort_order_mask;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (column->SortOrder != -1 && !column->IsEnabled)
                column->SortOrder = -1;
            if (column->SortOrder == -1)
                continue;
            sort_order_count++;
            sort_order_mask |= (ImGuiTableColumnMask(1) << column->SortOrder);
            IM_ASSERT(sort_order_count < (int)sizeof(sort_order_mask) * 8);
        }

        const bool need_fix_linearize = (ImGuiTableColumnMask(1) << sort_order_count) != (sort_order_mask + 1);
        const bool need_fix_single_sort_order = (sort_order_count > 1) && !(table->Flags & ImGuiTableFlags_SortMulti);
        if (need_fix_linearize || need_fix_single_sort_order)
        {
            ImGuiTableColumnMask fixed_mask;
            for (int sort_n = 0; sort_n < sort_order_count; sort_n++)
            {
                // Fix: Rewrite sort order fields if needed so they have no gap or duplicate.
                // (e.g. SortOrder 0 disappeared, SortOrder 1..2 exists --> rewrite then as SortOrder 0..1)
                int column_with_smallest_sort_order = -1;
                for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                    if ((fixed_mask & (ImGuiTableColumnMask(1) << column_n)) == 0 && table->Columns[column_n].SortOrder != -1)
                        if (column_with_smallest_sort_order == -1 || table->Columns[column_n].SortOrder < table->Columns[column_with_smallest_sort_order].SortOrder)
                            column_with_smallest_sort_order = column_n;
                IM_ASSERT(column_with_smallest_sort_order != -1);
                fixed_mask |= (ImGuiTableColumnMask(1) << column_with_smallest_sort_order);
                table->Columns[column_with_smallest_sort_order].SortOrder = (ImGuiTableColumnIdx)sort_n;

                // Fix: Make sure only one column has a SortOrder if ImGuiTableFlags_MultiSortable is not set.
                if (need_fix_single_sort_order)
                {
                    sort_order_count = 1;
                    for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
                        if (column_n != column_with_smallest_sort_order)
                            table->Columns[column_n].SortOrder = -1;
                    break;
                }
            }
        }

        // Fallback default sort order (if no column had the ImGuiTableColumnFlags_DefaultSort flag)
        if (sort_order_count == 0 && !(table->Flags & ImGuiTableFlags_SortTristate))
            for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            {
                ImGuiTableColumn* column = &table->Columns[column_n];
                if (column->IsEnabled && !(column->Flags & ImGuiTableColumnFlags_NoSort))
                {
                    sort_order_count = 1;
                    column->SortOrder = 0;
                    column->SortDirection = (ImU8)TableGetColumnAvailSortDirection(column, 0);
                    break;
                }
            }

        table->SortSpecsCount = (ImGuiTableColumnIdx)sort_order_count;
    }

    void TableSortSpecsBuild(ImGuiTable* table)
    {
        IM_ASSERT(table->IsSortSpecsDirty);
        TableSortSpecsSanitize(table);

        // Write output
        table->SortSpecsMulti.resize(table->SortSpecsCount <= 1 ? 0 : table->SortSpecsCount);
        ImGuiTableColumnSortSpecs* sort_specs = (table->SortSpecsCount == 0) ? NULL : (table->SortSpecsCount == 1) ? &table->SortSpecsSingle : table->SortSpecsMulti.Data;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (column->SortOrder == -1)
                continue;
            IM_ASSERT(column->SortOrder < table->SortSpecsCount);
            ImGuiTableColumnSortSpecs* sort_spec = &sort_specs[column->SortOrder];
            sort_spec->ColumnUserID = column->UserID;
            sort_spec->ColumnIndex = (ImGuiTableColumnIdx)column_n;
            sort_spec->SortOrder = (ImGuiTableColumnIdx)column->SortOrder;
            sort_spec->SortDirection = column->SortDirection;
        }
        table->SortSpecs.Specs = sort_specs;
        table->SortSpecs.SpecsCount = table->SortSpecsCount;
        table->SortSpecs.SpecsDirty = true; // Mark as dirty for user
        table->IsSortSpecsDirty = false; // Mark as not dirty for us
    }

    // Allocate draw channels. Called by TableUpdateLayout()
    // - We allocate them following storage order instead of display order so reordering columns won't needlessly
    //   increase overall dormant memory cost.
    // - We isolate headers draw commands in their own channels instead of just altering clip rects.
    //   This is in order to facilitate merging of draw commands.
    // - After crossing FreezeRowsCount, all columns see their current draw channel changed to a second set of channels.
    // - We only use the dummy draw channel so we can push a null clipping rectangle into it without affecting other
    //   channels, while simplifying per-row/per-cell overhead. It will be empty and discarded when merged.
    // - We allocate 1 or 2 background draw channels. This is because we know TablePushBackgroundChannel() is only used for
    //   horizontal spanning. If we allowed vertical spanning we'd need one background draw channel per merge group (1-4).
    // Draw channel allocation (before merging):
    // - NoClip                       --> 2+D+1 channels: bg0/1 + bg2 + foreground (same clip rect == always 1 draw call)
    // - Clip                         --> 2+D+N channels
    // - FreezeRows                   --> 2+D+N*2 (unless scrolling value is zero)
    // - FreezeRows || FreezeColunns  --> 3+D+N*2 (unless scrolling value is zero)
    // Where D is 1 if any column is clipped or hidden (dummy channel) otherwise 0.
    void TableSetupDrawChannels(ImGuiTable* table)
    {
        const int freeze_row_multiplier = (table->FreezeRowsCount > 0) ? 2 : 1;
        const int channels_for_row = (table->Flags & ImGuiTableFlags_NoClip) ? 1 : table->ColumnsEnabledCount;
        const int channels_for_bg = 1 + 1 * freeze_row_multiplier;
        const int channels_for_dummy = (table->ColumnsEnabledCount < table->ColumnsCount || table->VisibleMaskByIndex != table->EnabledMaskByIndex) ? +1 : 0;
        const int channels_total = channels_for_bg + (channels_for_row * freeze_row_multiplier) + channels_for_dummy;
        table->DrawSplitter.Split(table->InnerWindow->DrawList, channels_total);
        table->DummyDrawChannel = (ImGuiTableDrawChannelIdx)((channels_for_dummy > 0) ? channels_total - 1 : -1);
        table->Bg2DrawChannelCurrent = TABLE_DRAW_CHANNEL_BG2_FROZEN;
        table->Bg2DrawChannelUnfrozen = (ImGuiTableDrawChannelIdx)((table->FreezeRowsCount > 0) ? 2 + channels_for_row : TABLE_DRAW_CHANNEL_BG2_FROZEN);

        int draw_channel_current = 2;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (column->IsVisibleX && column->IsVisibleY)
            {
                column->DrawChannelFrozen = (ImGuiTableDrawChannelIdx)(draw_channel_current);
                column->DrawChannelUnfrozen = (ImGuiTableDrawChannelIdx)(draw_channel_current + (table->FreezeRowsCount > 0 ? channels_for_row + 1 : 0));
                if (!(table->Flags & ImGuiTableFlags_NoClip))
                    draw_channel_current++;
            }
            else
            {
                column->DrawChannelFrozen = column->DrawChannelUnfrozen = table->DummyDrawChannel;
            }
            column->DrawChannelCurrent = column->DrawChannelFrozen;
        }

        // Initial draw cmd starts with a BgClipRect that matches the one of its host, to facilitate merge draw commands by default.
        // All our cell highlight are manually clipped with BgClipRect. When unfreezing it will be made smaller to fit scrolling rect.
        // (This technically isn't part of setting up draw channels, but is reasonably related to be done here)
        table->BgClipRect = table->InnerClipRect;
        table->Bg0ClipRectForDrawCmd = table->OuterWindow->ClipRect;
        table->Bg2ClipRectForDrawCmd = table->HostClipRect;
        IM_ASSERT(table->BgClipRect.Min.y <= table->BgClipRect.Max.y);
    }

    // Return the resizing ID for the right-side of the given column.
    ImGuiID TableGetColumnResizeID(const ImGuiTable* table, int column_n, int instance_no)
    {
        IM_ASSERT(column_n >= 0 && column_n < table->ColumnsCount);
        ImGuiID id = table->ID + 1 + (instance_no * table->ColumnsCount) + column_n;
        return id;
    }

    // Disable clipping then auto-fit, will take 2 frames
   // (we don't take a shortcut for unclipped columns to reduce inconsistencies when e.g. resizing multiple columns)
    void TableSetColumnWidthAutoSingle(ImGuiTable* table, int column_n)
    {
        // Single auto width uses auto-fit
        ImGuiTableColumn* column = &table->Columns[column_n];
        if (!column->IsEnabled)
            return;
        column->CannotSkipItemsQueue = (1 << 0);
        table->AutoFitSingleColumn = (ImGuiTableColumnIdx)column_n;
    }

    // Process hit-testing on resizing borders. Actual size change will be applied in EndTable()
    // - Set table->HoveredColumnBorder with a short delay/timer to reduce feedback noise
    // - Submit ahead of table contents and header, use ImGuiButtonFlags_AllowItemOverlap to prioritize widgets
    //   overlapping the same area.
    void TableUpdateBorders(ImGuiTable* table)
    {
        ImGuiContext& g = *GImGui;
        IM_ASSERT(table->Flags & ImGuiTableFlags_Resizable);

        // At this point OuterRect height may be zero or under actual final height, so we rely on temporal coherency and
        // use the final height from last frame. Because this is only affecting _interaction_ with columns, it is not
        // really problematic (whereas the actual visual will be displayed in EndTable() and using the current frame height).
        // Actual columns highlight/render will be performed in EndTable() and not be affected.
        const float hit_half_width = TABLE_RESIZE_SEPARATOR_HALF_THICKNESS;
        const float hit_y1 = table->OuterRect.Min.y;
        const float hit_y2_body = ImMax(table->OuterRect.Max.y, hit_y1 + table->LastOuterHeight);
        const float hit_y2_head = hit_y1 + table->LastFirstRowHeight;

        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        {
            if (!(table->EnabledMaskByDisplayOrder & (ImGuiTableColumnMask(1) << order_n)))
                continue;

            const int column_n = table->DisplayOrderToIndex[order_n];
            ImGuiTableColumn* column = &table->Columns[column_n];
            if (column->Flags & (ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoDirectResize_))
                continue;

            // ImGuiTableFlags_NoBordersInBodyUntilResize will be honored in TableDrawBorders()
            const float border_y2_hit = (table->Flags & ImGuiTableFlags_NoBordersInBody) ? hit_y2_head : hit_y2_body;
            if ((table->Flags & ImGuiTableFlags_NoBordersInBody) && table->IsUsingHeaders == false)
                continue;

            if (table->FreezeColumnsCount > 0)
                if (column->MaxX < table->Columns[table->DisplayOrderToIndex[table->FreezeColumnsCount - 1]].MaxX)
                    continue;

            ImGuiID column_id = TableGetColumnResizeID(table, column_n, table->InstanceCurrent);
            ImRect hit_rect(column->MaxX - hit_half_width, hit_y1, column->MaxX + hit_half_width, border_y2_hit);
            //GetForegroundDrawList()->AddRect(hit_rect.Min, hit_rect.Max, IM_COL32(255, 0, 0, 100));
            ImGui::KeepAliveID(column_id);

            bool hovered = false, held = false;
            bool pressed = ImGui::ButtonBehavior(hit_rect, column_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDoubleClick);
            if (pressed && ImGui::IsMouseDoubleClicked(0))
            {
                TableSetColumnWidthAutoSingle(table, column_n);
                ImGui::ClearActiveID();
                held = hovered = false;
            }
            if (held)
            {
                if (table->LastResizedColumn == -1)
                    table->ResizeLockMinContentsX2 = table->RightMostEnabledColumn != -1 ? table->Columns[table->RightMostEnabledColumn].MaxX : -FLT_MAX;
                table->ResizedColumn = (ImGuiTableColumnIdx)column_n;
                table->InstanceInteracted = table->InstanceCurrent;
            }
            if ((hovered && g.HoveredIdTimer > TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER) || held)
            {
                table->HoveredColumnBorder = (ImGuiTableColumnIdx)column_n;
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }
        }
    }

    // Layout columns for the frame. This is in essence the followup to BeginTable().
    // Runs on the first call to TableNextRow(), to give a chance for TableSetupColumn() to be called first.
    // FIXME-TABLE: Our width (and therefore our WorkRect) will be minimal in the first frame for _WidthAuto columns.
    // Increase feedback side-effect with widgets relying on WorkRect.Max.x... Maybe provide a default distribution for _WidthAuto columns?
    void TableUpdateLayout(ImGuiTable* table)
    {
        ImGuiContext& g = *GImGui;
        IM_ASSERT(table->IsLayoutLocked == false);

        const ImGuiTableFlags table_sizing_policy = (table->Flags & ImGuiTableFlags_SizingMask_);
        table->IsDefaultDisplayOrder = true;
        table->ColumnsEnabledCount = 0;
        table->EnabledMaskByIndex.clear();
        table->EnabledMaskByDisplayOrder.clear();
        table->MinColumnWidth = ImMax(1.0f, g.Style.FramePadding.x * 1.0f); // g.Style.ColumnsMinSpacing; // FIXME-TABLE

        // [Part 1] Apply/lock Enabled and Order states. Calculate auto/ideal width for columns. Count fixed/stretch columns.
        // Process columns in their visible orders as we are building the Prev/Next indices.
        int count_fixed = 0;                // Number of columns that have fixed sizing policies
        int count_stretch = 0;              // Number of columns that have stretch sizing policies
        int last_visible_column_idx = -1;
        bool has_auto_fit_request = false;
        bool has_resizable = false;
        float stretch_sum_width_auto = 0.0f;
        float fixed_max_width_auto = 0.0f;
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        {
            const int column_n = table->DisplayOrderToIndex[order_n];
            if (column_n != order_n)
                table->IsDefaultDisplayOrder = false;
            ImGuiTableColumn* column = &table->Columns[column_n];

            // Clear column setup if not submitted by user. Currently we make it mandatory to call TableSetupColumn() every frame.
            // It would easily work without but we're not ready to guarantee it since e.g. names need resubmission anyway.
            // We take a slight shortcut but in theory we could be calling TableSetupColumn() here with dummy values, it should yield the same effect.
            if (table->DeclColumnsCount <= column_n)
            {
                TableSetupColumnFlags(table, column, ImGuiTableColumnFlags_None);
                column->NameOffset = -1;
                column->UserID = 0;
                column->InitStretchWeightOrWidth = -1.0f;
            }

            // Update Enabled state, mark settings/sortspecs dirty
            if (!(table->Flags & ImGuiTableFlags_Hideable) || (column->Flags & ImGuiTableColumnFlags_NoHide))
                column->IsEnabledNextFrame = true;
            if (column->IsEnabled != column->IsEnabledNextFrame)
            {
                column->IsEnabled = column->IsEnabledNextFrame;
                table->IsSettingsDirty = true;
                if (!column->IsEnabled && column->SortOrder != -1)
                    table->IsSortSpecsDirty = true;
            }
            if (column->SortOrder > 0 && !(table->Flags & ImGuiTableFlags_SortMulti))
                table->IsSortSpecsDirty = true;

            // Auto-fit unsized columns
            const bool start_auto_fit = (column->Flags & ImGuiTableColumnFlags_WidthFixed) ? (column->WidthRequest < 0.0f) : (column->StretchWeight < 0.0f);
            if (start_auto_fit)
                column->AutoFitQueue = column->CannotSkipItemsQueue = (1 << 3) - 1; // Fit for three frames

            if (!column->IsEnabled)
            {
                column->IndexWithinEnabledSet = -1;
                continue;
            }

            // Mark as enabled and link to previous/next enabled column
            column->PrevEnabledColumn = (ImGuiTableColumnIdx)last_visible_column_idx;
            column->NextEnabledColumn = -1;
            if (last_visible_column_idx != -1)
                table->Columns[last_visible_column_idx].NextEnabledColumn = (ImGuiTableColumnIdx)column_n;
            column->IndexWithinEnabledSet = table->ColumnsEnabledCount++;
            table->EnabledMaskByIndex |= ImGuiTableColumnMask(1) << column_n;
            table->EnabledMaskByDisplayOrder |= ImGuiTableColumnMask(1) << column->DisplayOrder;
            last_visible_column_idx = column_n;
            IM_ASSERT(column->IndexWithinEnabledSet <= column->DisplayOrder);

            // Calculate ideal/auto column width (that's the width required for all contents to be visible without clipping)
            // Combine width from regular rows + width from headers unless requested not to.
            if (!column->IsPreserveWidthAuto)
                column->WidthAuto = TableGetColumnWidthAuto(table, column);

            // Non-resizable columns keep their requested width (apply user value regardless of IsPreserveWidthAuto)
            const bool column_is_resizable = (column->Flags & ImGuiTableColumnFlags_NoResize) == 0;
            if (column_is_resizable)
                has_resizable = true;
            if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f && !column_is_resizable)
                column->WidthAuto = column->InitStretchWeightOrWidth;

            if (column->AutoFitQueue != 0x00)
                has_auto_fit_request = true;
            if (column->Flags & ImGuiTableColumnFlags_WidthStretch)
            {
                stretch_sum_width_auto += column->WidthAuto;
                count_stretch++;
            }
            else
            {
                fixed_max_width_auto = ImMax(fixed_max_width_auto, column->WidthAuto);
                count_fixed++;
            }
        }
        if ((table->Flags & ImGuiTableFlags_Sortable) && table->SortSpecsCount == 0 && !(table->Flags & ImGuiTableFlags_SortTristate))
            table->IsSortSpecsDirty = true;
        table->RightMostEnabledColumn = (ImGuiTableColumnIdx)last_visible_column_idx;
        IM_ASSERT(table->RightMostEnabledColumn >= 0);

        // [Part 2] Disable child window clipping while fitting columns. This is not strictly necessary but makes it possible
        // to avoid the column fitting having to wait until the first visible frame of the child container (may or not be a good thing).
        // FIXME-TABLE: for always auto-resizing columns may not want to do that all the time.
        if (has_auto_fit_request && table->OuterWindow != table->InnerWindow)
            table->InnerWindow->SkipItems = false;
        if (has_auto_fit_request)
            table->IsSettingsDirty = true;

        // [Part 3] Fix column flags and record a few extra information.
        float sum_width_requests = 0.0f;        // Sum of all width for fixed and auto-resize columns, excluding width contributed by Stretch columns but including spacing/padding.
        float stretch_sum_weights = 0.0f;       // Sum of all weights for stretch columns.
        table->LeftMostStretchedColumn = table->RightMostStretchedColumn = -1;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            if (!(table->EnabledMaskByIndex & (ImGuiTableColumnMask(1) << column_n)))
                continue;
            ImGuiTableColumn* column = &table->Columns[column_n];

            const bool column_is_resizable = (column->Flags & ImGuiTableColumnFlags_NoResize) == 0;
            if (column->Flags & ImGuiTableColumnFlags_WidthFixed)
            {
                // Apply same widths policy
                float width_auto = column->WidthAuto;
                if (table_sizing_policy == ImGuiTableFlags_SizingFixedSame && (column->AutoFitQueue != 0x00 || !column_is_resizable))
                    width_auto = fixed_max_width_auto;

                // Apply automatic width
                // Latch initial size for fixed columns and update it constantly for auto-resizing column (unless clipped!)
                if (column->AutoFitQueue != 0x00)
                    column->WidthRequest = width_auto;
                else if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && !column_is_resizable && (table->RequestOutputMaskByIndex & (ImGuiTableColumnMask(1) << column_n)))
                    column->WidthRequest = width_auto;

                // FIXME-TABLE: Increase minimum size during init frame to avoid biasing auto-fitting widgets
                // (e.g. TextWrapped) too much. Otherwise what tends to happen is that TextWrapped would output a very
                // large height (= first frame scrollbar display very off + clipper would skip lots of items).
                // This is merely making the side-effect less extreme, but doesn't properly fixes it.
                // FIXME: Move this to ->WidthGiven to avoid temporary lossyless?
                // FIXME: This break IsPreserveWidthAuto from not flickering if the stored WidthAuto was smaller.
                if (column->AutoFitQueue > 0x01 && table->IsInitializing && !column->IsPreserveWidthAuto)
                    column->WidthRequest = ImMax(column->WidthRequest, table->MinColumnWidth * 4.0f); // FIXME-TABLE: Another constant/scale?
                sum_width_requests += column->WidthRequest;
            }
            else
            {
                // Initialize stretch weight
                if (column->AutoFitQueue != 0x00 || column->StretchWeight < 0.0f || !column_is_resizable)
                {
                    if (column->InitStretchWeightOrWidth > 0.0f)
                        column->StretchWeight = column->InitStretchWeightOrWidth;
                    else if (table_sizing_policy == ImGuiTableFlags_SizingStretchProp)
                        column->StretchWeight = (column->WidthAuto / stretch_sum_width_auto) * count_stretch;
                    else
                        column->StretchWeight = 1.0f;
                }

                stretch_sum_weights += column->StretchWeight;
                if (table->LeftMostStretchedColumn == -1 || table->Columns[table->LeftMostStretchedColumn].DisplayOrder > column->DisplayOrder)
                    table->LeftMostStretchedColumn = (ImGuiTableColumnIdx)column_n;
                if (table->RightMostStretchedColumn == -1 || table->Columns[table->RightMostStretchedColumn].DisplayOrder < column->DisplayOrder)
                    table->RightMostStretchedColumn = (ImGuiTableColumnIdx)column_n;
            }
            column->IsPreserveWidthAuto = false;
            sum_width_requests += table->CellPaddingX * 2.0f;
        }
        table->ColumnsEnabledFixedCount = (ImGuiTableColumnIdx)count_fixed;

        // [Part 4] Apply final widths based on requested widths
        const ImRect work_rect = table->WorkRect;
        const float width_spacings = (table->OuterPaddingX * 2.0f) + (table->CellSpacingX1 + table->CellSpacingX2) * (table->ColumnsEnabledCount - 1);
        const float width_avail = ((table->Flags & ImGuiTableFlags_ScrollX) && table->InnerWidth == 0.0f) ? table->InnerClipRect.GetWidth() : work_rect.GetWidth();
        const float width_avail_for_stretched_columns = width_avail - width_spacings - sum_width_requests;
        float width_remaining_for_stretched_columns = width_avail_for_stretched_columns;
        table->ColumnsGivenWidth = width_spacings + (table->CellPaddingX * 2.0f) * table->ColumnsEnabledCount;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            if (!(table->EnabledMaskByIndex & (ImGuiTableColumnMask(1) << column_n)))
                continue;
            ImGuiTableColumn* column = &table->Columns[column_n];

            // Allocate width for stretched/weighted columns (StretchWeight gets converted into WidthRequest)
            if (column->Flags & ImGuiTableColumnFlags_WidthStretch)
            {
                float weight_ratio = column->StretchWeight / stretch_sum_weights;
                column->WidthRequest = IM_FLOOR(ImMax(width_avail_for_stretched_columns * weight_ratio, table->MinColumnWidth) + 0.01f);
                width_remaining_for_stretched_columns -= column->WidthRequest;
            }

            // [Resize Rule 1] The right-most Visible column is not resizable if there is at least one Stretch column
            // See additional comments in TableSetColumnWidth().
            if (column->NextEnabledColumn == -1 && table->LeftMostStretchedColumn != -1)
                column->Flags |= ImGuiTableColumnFlags_NoDirectResize_;

            // Assign final width, record width in case we will need to shrink
            column->WidthGiven = ImFloor(ImMax(column->WidthRequest, table->MinColumnWidth));
            table->ColumnsGivenWidth += column->WidthGiven;
        }

        // [Part 5] Redistribute stretch remainder width due to rounding (remainder width is < 1.0f * number of Stretch column).
        // Using right-to-left distribution (more likely to match resizing cursor).
        if (width_remaining_for_stretched_columns >= 1.0f && !(table->Flags & ImGuiTableFlags_PreciseWidths))
            for (int order_n = table->ColumnsCount - 1; stretch_sum_weights > 0.0f && width_remaining_for_stretched_columns >= 1.0f && order_n >= 0; order_n--)
            {
                if (!(table->EnabledMaskByDisplayOrder & (ImGuiTableColumnMask(1) << order_n)))
                    continue;
                ImGuiTableColumn* column = &table->Columns[table->DisplayOrderToIndex[order_n]];
                if (!(column->Flags & ImGuiTableColumnFlags_WidthStretch))
                    continue;
                column->WidthRequest += 1.0f;
                column->WidthGiven += 1.0f;
                width_remaining_for_stretched_columns -= 1.0f;
            }

        table->HoveredColumnBody = -1;
        table->HoveredColumnBorder = -1;
        const ImRect mouse_hit_rect(table->OuterRect.Min.x, table->OuterRect.Min.y, table->OuterRect.Max.x, ImMax(table->OuterRect.Max.y, table->OuterRect.Min.y + table->LastOuterHeight));
        const bool is_hovering_table = ImGui::ItemHoverable(mouse_hit_rect, 0);

        // [Part 6] Setup final position, offset, skip/clip states and clipping rectangles, detect hovered column
        // Process columns in their visible orders as we are comparing the visible order and adjusting host_clip_rect while looping.
        int visible_n = 0;
        bool offset_x_frozen = (table->FreezeColumnsCount > 0);
        float offset_x = ((table->FreezeColumnsCount > 0) ? table->OuterRect.Min.x : work_rect.Min.x) + table->OuterPaddingX - table->CellSpacingX1;
        ImRect host_clip_rect = table->InnerClipRect;
        //host_clip_rect.Max.x += table->CellPaddingX + table->CellSpacingX2;
        table->VisibleMaskByIndex.clear();
        table->RequestOutputMaskByIndex.clear();
        for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
        {
            const int column_n = table->DisplayOrderToIndex[order_n];
            ImGuiTableColumn* column = &table->Columns[column_n];

            column->NavLayerCurrent = (ImS8)((table->FreezeRowsCount > 0 || column_n < table->FreezeColumnsCount) ? ImGuiNavLayer_Menu : ImGuiNavLayer_Main);

            if (offset_x_frozen && table->FreezeColumnsCount == visible_n)
            {
                offset_x += work_rect.Min.x - table->OuterRect.Min.x;
                offset_x_frozen = false;
            }

            // Clear status flags
            column->Flags &= ~ImGuiTableColumnFlags_StatusMask_;

            if ((table->EnabledMaskByDisplayOrder & (ImGuiTableColumnMask(1) << order_n)) == 0)
            {
                // Hidden column: clear a few fields and we are done with it for the remainder of the function.
                // We set a zero-width clip rect but set Min.y/Max.y properly to not interfere with the clipper.
                column->MinX = column->MaxX = column->WorkMinX = column->ClipRect.Min.x = column->ClipRect.Max.x = offset_x;
                column->WidthGiven = 0.0f;
                column->ClipRect.Min.y = work_rect.Min.y;
                column->ClipRect.Max.y = FLT_MAX;
                column->ClipRect.ClipWithFull(host_clip_rect);
                column->IsVisibleX = column->IsVisibleY = column->IsRequestOutput = false;
                column->IsSkipItems = true;
                column->ItemWidth = 1.0f;
                continue;
            }

            // Detect hovered column
            if (is_hovering_table && g.IO.MousePos.x >= column->ClipRect.Min.x && g.IO.MousePos.x < column->ClipRect.Max.x)
                table->HoveredColumnBody = (ImGuiTableColumnIdx)column_n;

            // Lock start position
            column->MinX = offset_x;

            // Lock width based on start position and minimum/maximum width for this position
            float max_width = TableGetMaxColumnWidth(table, column_n);
            column->WidthGiven = ImMin(column->WidthGiven, max_width);
            column->WidthGiven = ImMax(column->WidthGiven, ImMin(column->WidthRequest, table->MinColumnWidth));
            column->MaxX = offset_x + column->WidthGiven + table->CellSpacingX1 + table->CellSpacingX2 + table->CellPaddingX * 2.0f;

            // Lock other positions
            // - ClipRect.Min.x: Because merging draw commands doesn't compare min boundaries, we make ClipRect.Min.x match left bounds to be consistent regardless of merging.
            // - ClipRect.Max.x: using WorkMaxX instead of MaxX (aka including padding) makes things more consistent when resizing down, tho slightly detrimental to visibility in very-small column.
            // - ClipRect.Max.x: using MaxX makes it easier for header to receive hover highlight with no discontinuity and display sorting arrow.
            // - FIXME-TABLE: We want equal width columns to have equal (ClipRect.Max.x - WorkMinX) width, which means ClipRect.max.x cannot stray off host_clip_rect.Max.x else right-most column may appear shorter.
            column->WorkMinX = column->MinX + table->CellPaddingX + table->CellSpacingX1;
            column->WorkMaxX = column->MaxX - table->CellPaddingX - table->CellSpacingX2; // Expected max
            column->ItemWidth = ImFloor(column->WidthGiven * 0.65f);
            column->ClipRect.Min.x = column->MinX;
            column->ClipRect.Min.y = work_rect.Min.y;
            column->ClipRect.Max.x = column->MaxX; //column->WorkMaxX;
            column->ClipRect.Max.y = FLT_MAX;
            column->ClipRect.ClipWithFull(host_clip_rect);

            // Mark column as Clipped (not in sight)
            // Note that scrolling tables (where inner_window != outer_window) handle Y clipped earlier in BeginTable() so IsVisibleY really only applies to non-scrolling tables.
            // FIXME-TABLE: Because InnerClipRect.Max.y is conservatively ==outer_window->ClipRect.Max.y, we never can mark columns _Above_ the scroll line as not IsVisibleY.
            // Taking advantage of LastOuterHeight would yield good results there...
            // FIXME-TABLE: Y clipping is disabled because it effectively means not submitting will reduce contents width which is fed to outer_window->DC.CursorMaxPos.x,
            // and this may be used (e.g. typically by outer_window using AlwaysAutoResize or outer_window's horizontal scrollbar, but could be something else).
            // Possible solution to preserve last known content width for clipped column. Test 'table_reported_size' fails when enabling Y clipping and window is resized small.
            column->IsVisibleX = (column->ClipRect.Max.x > column->ClipRect.Min.x);
            column->IsVisibleY = true; // (column->ClipRect.Max.y > column->ClipRect.Min.y);
            const bool is_visible = column->IsVisibleX; //&& column->IsVisibleY;
            if (is_visible)
                table->VisibleMaskByIndex |= (ImGuiTableColumnMask(1) << column_n);

            // Mark column as requesting output from user. Note that fixed + non-resizable sets are auto-fitting at all times and therefore always request output.
            column->IsRequestOutput = is_visible || column->AutoFitQueue != 0 || column->CannotSkipItemsQueue != 0;
            if (column->IsRequestOutput)
                table->RequestOutputMaskByIndex |= (ImGuiTableColumnMask(1) << column_n);

            // Mark column as SkipItems (ignoring all items/layout)
            column->IsSkipItems = !column->IsEnabled || table->HostSkipItems;
            if (column->IsSkipItems) {
                IM_ASSERT(!is_visible);
            }

            // Update status flags
            column->Flags |= ImGuiTableColumnFlags_IsEnabled;
            if (is_visible)
                column->Flags |= ImGuiTableColumnFlags_IsVisible;
            if (column->SortOrder != -1)
                column->Flags |= ImGuiTableColumnFlags_IsSorted;
            if (table->HoveredColumnBody == column_n)
                column->Flags |= ImGuiTableColumnFlags_IsHovered;

            // Alignment
            // FIXME-TABLE: This align based on the whole column width, not per-cell, and therefore isn't useful in
            // many cases (to be able to honor this we might be able to store a log of cells width, per row, for
            // visible rows, but nav/programmatic scroll would have visible artifacts.)
            //if (column->Flags & ImGuiTableColumnFlags_AlignRight)
            //    column->WorkMinX = ImMax(column->WorkMinX, column->MaxX - column->ContentWidthRowsUnfrozen);
            //else if (column->Flags & ImGuiTableColumnFlags_AlignCenter)
            //    column->WorkMinX = ImLerp(column->WorkMinX, ImMax(column->StartX, column->MaxX - column->ContentWidthRowsUnfrozen), 0.5f);

            // Reset content width variables
            column->ContentMaxXFrozen = column->ContentMaxXUnfrozen = column->WorkMinX;
            column->ContentMaxXHeadersUsed = column->ContentMaxXHeadersIdeal = column->WorkMinX;

            // Don't decrement auto-fit counters until container window got a chance to submit its items
            if (table->HostSkipItems == false)
            {
                column->AutoFitQueue >>= 1;
                column->CannotSkipItemsQueue >>= 1;
            }

            if (visible_n < table->FreezeColumnsCount)
                host_clip_rect.Min.x = ImClamp(column->MaxX + TABLE_BORDER_SIZE, host_clip_rect.Min.x, host_clip_rect.Max.x);

            offset_x += column->WidthGiven + table->CellSpacingX1 + table->CellSpacingX2 + table->CellPaddingX * 2.0f;
            visible_n++;
        }

        // [Part 7] Detect/store when we are hovering the unused space after the right-most column (so e.g. context menus can react on it)
        // Clear Resizable flag if none of our column are actually resizable (either via an explicit _NoResize flag, either
        // because of using _WidthAuto/_WidthStretch). This will hide the resizing option from the context menu.
        const float unused_x1 = ImMax(table->WorkRect.Min.x, table->Columns[table->RightMostEnabledColumn].ClipRect.Max.x);
        if (is_hovering_table && table->HoveredColumnBody == -1)
        {
            if (g.IO.MousePos.x >= unused_x1)
                table->HoveredColumnBody = (ImGuiTableColumnIdx)table->ColumnsCount;
        }
        if (has_resizable == false && (table->Flags & ImGuiTableFlags_Resizable))
            table->Flags &= ~ImGuiTableFlags_Resizable;

        // [Part 8] Lock actual OuterRect/WorkRect right-most position.
        // This is done late to handle the case of fixed-columns tables not claiming more widths that they need.
        // Because of this we are careful with uses of WorkRect and InnerClipRect before this point.
        if (table->RightMostStretchedColumn != -1)
            table->Flags &= ~ImGuiTableFlags_NoHostExtendX;
        if (table->Flags & ImGuiTableFlags_NoHostExtendX)
        {
            table->OuterRect.Max.x = table->WorkRect.Max.x = unused_x1;
            table->InnerClipRect.Max.x = ImMin(table->InnerClipRect.Max.x, unused_x1);
        }
        table->InnerWindow->ParentWorkRect = table->WorkRect;
        table->BorderX1 = table->InnerClipRect.Min.x;// +((table->Flags & ImGuiTableFlags_BordersOuter) ? 0.0f : -1.0f);
        table->BorderX2 = table->InnerClipRect.Max.x;// +((table->Flags & ImGuiTableFlags_BordersOuter) ? 0.0f : +1.0f);

        // [Part 9] Allocate draw channels and setup background cliprect
        TableSetupDrawChannels(table);

        // [Part 10] Hit testing on borders
        if (table->Flags & ImGuiTableFlags_Resizable)
            TableUpdateBorders(table);
        table->LastFirstRowHeight = 0.0f;
        table->IsLayoutLocked = true;
        table->IsUsingHeaders = false;

        // [Part 11] Context menu
        // FIXME knox: This is disabled cause my plugins don't use the native table context menu
        // if (table->IsContextPopupOpen && table->InstanceCurrent == table->InstanceInteracted)
        // {
        //     const ImGuiID context_menu_id = ImHashStr("##ContextMenu", 0, table->ID);
        //     if (ImGui::BeginPopupEx(context_menu_id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
        //     {
        //         TableDrawContextMenu(table);
        //         EndPopup();
        //     }
        //     else
        //     {
        //         table->IsContextPopupOpen = false;
        //     }
        // }

        // [Part 13] Sanitize and build sort specs before we have a change to use them for display.
        // This path will only be exercised when sort specs are modified before header rows (e.g. init or visibility change)
        if (table->IsSortSpecsDirty && (table->Flags & ImGuiTableFlags_Sortable))
            TableSortSpecsBuild(table);

        // Initial state
        ImGuiWindow* inner_window = table->InnerWindow;
        if (table->Flags & ImGuiTableFlags_NoClip)
            table->DrawSplitter.SetCurrentChannel(inner_window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
        else
            inner_window->DrawList->PushClipRect(inner_window->ClipRect.Min, inner_window->ClipRect.Max, false);
    }

    // [Public] Starts into the first cell of a new row
    void TableNextRow(ImGuiTableRowFlags row_flags, float row_min_height)
    {
        ImGuiTable* table = CurrentTable;

        if (!table->IsLayoutLocked)
            TableUpdateLayout(table);
        if (table->IsInsideRow)
            TableEndRow(table);

        table->LastRowFlags = table->RowFlags;
        table->RowFlags = row_flags;
        table->RowMinHeight = row_min_height;
        TableBeginRow(table);

        // We honor min_row_height requested by user, but cannot guarantee per-row maximum height,
        // because that would essentially require a unique clipping rectangle per-cell.
        table->RowPosY2 += table->CellPaddingY * 2.0f;
        table->RowPosY2 = ImMax(table->RowPosY2, table->RowPosY1 + row_min_height);

        // Disable output until user calls TableNextColumn()
        table->InnerWindow->SkipItems = true;
    }

    // Note this is meant to be stored in column->WidthAuto, please generally use the WidthAuto field
    float TableGetColumnWidthAuto(ImGuiTable* table, ImGuiTableColumn* column)
    {
        const float content_width_body = ImMax(column->ContentMaxXFrozen, column->ContentMaxXUnfrozen) - column->WorkMinX;
        const float content_width_headers = column->ContentMaxXHeadersIdeal - column->WorkMinX;
        float width_auto = content_width_body;
        if (!(column->Flags & ImGuiTableColumnFlags_NoHeaderWidth))
            width_auto = ImMax(width_auto, content_width_headers);

        // Non-resizable fixed columns preserve their requested width
        if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f)
            if (!(table->Flags & ImGuiTableFlags_Resizable) || (column->Flags & ImGuiTableColumnFlags_NoResize))
                width_auto = column->InitStretchWeightOrWidth;

        return ImMax(width_auto, table->MinColumnWidth);
    }

    // [Internal] Called by TableSetColumnIndex()/TableNextColumn()
	// This is called very frequently, so we need to be mindful of unnecessary overhead.
	// FIXME-TABLE FIXME-OPT: Could probably shortcut some things for non-active or clipped columns.
    void TableBeginCell(ImGuiTable* table, int column_n)
    {
        ImGuiTableColumn* column = &table->Columns[column_n];
        ImGuiWindow* window = table->InnerWindow;
        table->CurrentColumn = column_n;

        // Start position is roughly ~~ CellRect.Min + CellPadding + Indent
        float start_x = column->WorkMinX;
        if (column->Flags & ImGuiTableColumnFlags_IndentEnable)
            start_x += table->RowIndentOffsetX; // ~~ += window.DC.Indent.x - table->HostIndentX, except we locked it for the row.

        window->DC.CursorPos.x = start_x;
        window->DC.CursorPos.y = table->RowPosY1 + table->CellPaddingY;
        window->DC.CursorMaxPos.x = window->DC.CursorPos.x;
        window->DC.ColumnsOffset.x = start_x - window->Pos.x - window->DC.Indent.x; // FIXME-WORKRECT
        window->DC.CurrLineTextBaseOffset = table->RowTextBaseline;
        window->DC.NavLayerCurrent = (ImGuiNavLayer)column->NavLayerCurrent;

        window->WorkRect.Min.y = window->DC.CursorPos.y;
        window->WorkRect.Min.x = column->WorkMinX;
        window->WorkRect.Max.x = column->WorkMaxX;
        window->DC.ItemWidth = column->ItemWidth;

        // To allow ImGuiListClipper to function we propagate our row height
        if (!column->IsEnabled)
            window->DC.CursorPos.y = ImMax(window->DC.CursorPos.y, table->RowPosY2);

        window->SkipItems = column->IsSkipItems;
        if (column->IsSkipItems)
        {
            window->DC.LastItemId = 0;
            window->DC.LastItemStatusFlags = 0;
        }

        if (table->Flags & ImGuiTableFlags_NoClip)
        {
            // FIXME: if we end up drawing all borders/bg in EndTable, could remove this and just assert that channel hasn't changed.
            table->DrawSplitter.SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
            //IM_ASSERT(table->DrawSplitter._Current == TABLE_DRAW_CHANNEL_NOCLIP);
        }
        else
        {
            // FIXME-TABLE: Could avoid this if draw channel is dummy channel?
            SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
            table->DrawSplitter.SetCurrentChannel(window->DrawList, column->DrawChannelCurrent);
        }
    }

    // [Public] Append into the next column, wrap and create a new row when already on last column
    bool TableNextColumn()
    {
        ImGuiTable* table = CurrentTable;
        if (!table)
            return false;

        if (table->IsInsideRow && table->CurrentColumn + 1 < table->ColumnsCount)
        {
            if (table->CurrentColumn != -1)
                TableEndCell(table);
            TableBeginCell(table, table->CurrentColumn + 1);
        }
        else
        {
            TableNextRow();
            TableBeginCell(table, 0);
        }

        // Return whether the column is visible. User may choose to skip submitting items based on this return value,
        // however they shouldn't skip submitting for columns that may have the tallest contribution to row height.
        int column_n = table->CurrentColumn;
        return (table->RequestOutputMaskByIndex & (ImGuiTableColumnMask(1) << column_n)) != 0;
    }

    // Return NULL if no sort specs (most often when ImGuiTableFlags_Sortable is not set)
	// You can sort your data again when 'SpecsChanged == true'. It will be true with sorting specs have changed since
	// last call, or the first time.
	// Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable()!
    ImGuiTableSortSpecs* TableGetSortSpecs()
    {
        ImGuiTable* table = CurrentTable;
        IM_ASSERT(table != NULL);

        if (!(table->Flags & ImGuiTableFlags_Sortable))
            return NULL;

        // Require layout (in case TableHeadersRow() hasn't been called) as it may alter IsSortSpecsDirty in some paths.
        if (!table->IsLayoutLocked)
            TableUpdateLayout(table);

        if (table->IsSortSpecsDirty)
            TableSortSpecsBuild(table);

        return &table->SortSpecs;
    }

    // FIXME-TABLE: This is a mess, need to redesign how we render borders (as some are also done in TableEndRow)
    void TableDrawBorders(ImGuiTable* table)
    {
        ImGuiWindow* inner_window = table->InnerWindow;
        if (!table->OuterWindow->ClipRect.Overlaps(table->OuterRect))
            return;

        ImDrawList* inner_drawlist = inner_window->DrawList;
        table->DrawSplitter.SetCurrentChannel(inner_drawlist, TABLE_DRAW_CHANNEL_BG0);
        inner_drawlist->PushClipRect(table->Bg0ClipRectForDrawCmd.Min, table->Bg0ClipRectForDrawCmd.Max, false);

        // Draw inner border and resizing feedback
        const float border_size = TABLE_BORDER_SIZE;
        const float draw_y1 = table->InnerRect.Min.y;
        const float draw_y2_body = table->InnerRect.Max.y;
        const float draw_y2_head = table->IsUsingHeaders ? ImMin(table->InnerRect.Max.y, (table->FreezeRowsCount >= 1 ? table->InnerRect.Min.y : table->WorkRect.Min.y) + table->LastFirstRowHeight) : draw_y1;
        if (table->Flags & ImGuiTableFlags_BordersInnerV)
        {
            for (int order_n = 0; order_n < table->ColumnsCount; order_n++)
            {
                if (!(table->EnabledMaskByDisplayOrder & (ImGuiTableColumnMask(1) << order_n)))
                    continue;

                const int column_n = table->DisplayOrderToIndex[order_n];
                ImGuiTableColumn* column = &table->Columns[column_n];
                const bool is_hovered = (table->HoveredColumnBorder == column_n);
                const bool is_resized = (table->ResizedColumn == column_n) && (table->InstanceInteracted == table->InstanceCurrent);
                const bool is_resizable = (column->Flags & (ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoDirectResize_)) == 0;
                const bool is_frozen_separator = (table->FreezeColumnsCount != -1 && table->FreezeColumnsCount == order_n + 1);
                if (column->MaxX > table->InnerClipRect.Max.x && !is_resized)
                    continue;

                // Decide whether right-most column is visible
                if (column->NextEnabledColumn == -1 && !is_resizable)
                    if ((table->Flags & ImGuiTableFlags_SizingMask_) != ImGuiTableFlags_SizingFixedSame || (table->Flags & ImGuiTableFlags_NoHostExtendX))
                        continue;
                if (column->MaxX <= column->ClipRect.Min.x) // FIXME-TABLE FIXME-STYLE: Assume BorderSize==1, this is problematic if we want to increase the border size..
                    continue;

                // Draw in outer window so right-most column won't be clipped
                // Always draw full height border when being resized/hovered, or on the delimitation of frozen column scrolling.
                ImU32 col;
                float draw_y2;
                if (is_hovered || is_resized || is_frozen_separator)
                {
                    draw_y2 = draw_y2_body;
                    col = is_resized ? ImGui::GetColorU32(ImGuiCol_SeparatorActive) : is_hovered ? ImGui::GetColorU32(ImGuiCol_SeparatorHovered) : table->BorderColorStrong;
                }
                else
                {
                    draw_y2 = (table->Flags & (ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoBordersInBodyUntilResize)) ? draw_y2_head : draw_y2_body;
                    col = (table->Flags & (ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoBordersInBodyUntilResize)) ? table->BorderColorStrong : table->BorderColorLight;
                }

                if (draw_y2 > draw_y1)
                    inner_drawlist->AddLine(ImVec2(column->MaxX, draw_y1), ImVec2(column->MaxX, draw_y2), col, border_size);
            }
        }

        // Draw outer border
        // FIXME: could use AddRect or explicit VLine/HLine helper?
        if (table->Flags & ImGuiTableFlags_BordersOuter)
        {
            // Display outer border offset by 1 which is a simple way to display it without adding an extra draw call
            // (Without the offset, in outer_window it would be rendered behind cells, because child windows are above their
            // parent. In inner_window, it won't reach out over scrollbars. Another weird solution would be to display part
            // of it in inner window, and the part that's over scrollbars in the outer window..)
            // Either solution currently won't allow us to use a larger border size: the border would clipped.
            const ImRect outer_border = table->OuterRect;
            const ImU32 outer_col = table->BorderColorStrong;
            if ((table->Flags & ImGuiTableFlags_BordersOuter) == ImGuiTableFlags_BordersOuter)
            {
                inner_drawlist->AddRect(outer_border.Min, outer_border.Max, outer_col, 0.0f, ~0, border_size);
            }
            else if (table->Flags & ImGuiTableFlags_BordersOuterV)
            {
                inner_drawlist->AddLine(outer_border.Min, ImVec2(outer_border.Min.x, outer_border.Max.y), outer_col, border_size);
                inner_drawlist->AddLine(ImVec2(outer_border.Max.x, outer_border.Min.y), outer_border.Max, outer_col, border_size);
            }
            else if (table->Flags & ImGuiTableFlags_BordersOuterH)
            {
                inner_drawlist->AddLine(outer_border.Min, ImVec2(outer_border.Max.x, outer_border.Min.y), outer_col, border_size);
                inner_drawlist->AddLine(ImVec2(outer_border.Min.x, outer_border.Max.y), outer_border.Max, outer_col, border_size);
            }
        }
        if ((table->Flags & ImGuiTableFlags_BordersInnerH) && table->RowPosY2 < table->OuterRect.Max.y)
        {
            // Draw bottom-most row border
            const float border_y = table->RowPosY2;
            if (border_y >= table->BgClipRect.Min.y && border_y < table->BgClipRect.Max.y)
                inner_drawlist->AddLine(ImVec2(table->BorderX1, border_y), ImVec2(table->BorderX2, border_y), table->BorderColorLight, border_size);
        }

        inner_drawlist->PopClipRect();
    }

    // This function reorder draw channels based on matching clip rectangle, to facilitate merging them. Called by EndTable().
// For simplicity we call it TableMergeDrawChannels() but in fact it only reorder channels + overwrite ClipRect,
// actual merging is done by table->DrawSplitter.Merge() which is called right after TableMergeDrawChannels().
//
// Columns where the contents didn't stray off their local clip rectangle can be merged. To achieve
// this we merge their clip rect and make them contiguous in the channel list, so they can be merged
// by the call to DrawSplitter.Merge() following to the call to this function.
// We reorder draw commands by arranging them into a maximum of 4 distinct groups:
//
//   1 group:               2 groups:              2 groups:              4 groups:
//   [ 0. ] no freeze       [ 0. ] row freeze      [ 01 ] col freeze      [ 01 ] row+col freeze
//   [ .. ]  or no scroll   [ 2. ]  and v-scroll   [ .. ]  and h-scroll   [ 23 ]  and v+h-scroll
//
// Each column itself can use 1 channel (row freeze disabled) or 2 channels (row freeze enabled).
// When the contents of a column didn't stray off its limit, we move its channels into the corresponding group
// based on its position (within frozen rows/columns groups or not).
// At the end of the operation our 1-4 groups will each have a ImDrawCmd using the same ClipRect.
// This function assume that each column are pointing to a distinct draw channel,
// otherwise merge_group->ChannelsCount will not match set bit count of merge_group->ChannelsMask.
//
// Column channels will not be merged into one of the 1-4 groups in the following cases:
// - The contents stray off its clipping rectangle (we only compare the MaxX value, not the MinX value).
//   Direct ImDrawList calls won't be taken into account by default, if you use them make sure the ImGui:: bounds
//   matches, by e.g. calling SetCursorScreenPos().
// - The channel uses more than one draw command itself. We drop all our attempt at merging stuff here..
//   we could do better but it's going to be rare and probably not worth the hassle.
// Columns for which the draw channel(s) haven't been merged with other will use their own ImDrawCmd.
//
// This function is particularly tricky to understand.. take a breath.
    void TableMergeDrawChannels(ImGuiTable* table)
    {
        ImGuiContext& g = *GImGui;
        ImDrawListSplitter* splitter = &table->DrawSplitter;
        const bool has_freeze_v = (table->FreezeRowsCount > 0);
        const bool has_freeze_h = (table->FreezeColumnsCount > 0);
        IM_ASSERT(splitter->_Current == 0);

        // Track which groups we are going to attempt to merge, and which channels goes into each group.
        struct MergeGroup
        {
            ImRect  ClipRect;
            int     ChannelsCount;
            ImBitArray<IMGUIEX_TABLE_MAX_DRAW_CHANNELS> ChannelsMask;
        };
        int merge_group_mask = 0x00;
        MergeGroup merge_groups[4];
        memset(merge_groups, 0, sizeof(merge_groups));

        // 1. Scan channels and take note of those which can be merged
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
        {
            if ((table->VisibleMaskByIndex & (ImGuiTableColumnMask(1) << column_n)) == 0)
                continue;
            ImGuiTableColumn* column = &table->Columns[column_n];

            const int merge_group_sub_count = has_freeze_v ? 2 : 1;
            for (int merge_group_sub_n = 0; merge_group_sub_n < merge_group_sub_count; merge_group_sub_n++)
            {
                const int channel_no = (merge_group_sub_n == 0) ? column->DrawChannelFrozen : column->DrawChannelUnfrozen;

                // Don't attempt to merge if there are multiple draw calls within the column
                ImDrawChannel* src_channel = &splitter->_Channels[channel_no];
                if (src_channel->_CmdBuffer.Size > 0 && src_channel->_CmdBuffer.back().ElemCount == 0)
                    src_channel->_CmdBuffer.pop_back();
                if (src_channel->_CmdBuffer.Size != 1)
                    continue;

                // Find out the width of this merge group and check if it will fit in our column
                // (note that we assume that rendering didn't stray on the left direction. we should need a CursorMinPos to detect it)
                if (!(column->Flags & ImGuiTableColumnFlags_NoClip))
                {
                    float content_max_x;
                    if (!has_freeze_v)
                        content_max_x = ImMax(column->ContentMaxXUnfrozen, column->ContentMaxXHeadersUsed); // No row freeze
                    else if (merge_group_sub_n == 0)
                        content_max_x = ImMax(column->ContentMaxXFrozen, column->ContentMaxXHeadersUsed);   // Row freeze: use width before freeze
                    else
                        content_max_x = column->ContentMaxXUnfrozen;                                        // Row freeze: use width after freeze
                    if (content_max_x > column->ClipRect.Max.x)
                        continue;
                }

                const int merge_group_n = (has_freeze_h && column_n < table->FreezeColumnsCount ? 0 : 1) + (has_freeze_v && merge_group_sub_n == 0 ? 0 : 2);
                IM_ASSERT(channel_no < IMGUIEX_TABLE_MAX_DRAW_CHANNELS);
                MergeGroup* merge_group = &merge_groups[merge_group_n];
                if (merge_group->ChannelsCount == 0)
                    merge_group->ClipRect = ImRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
                merge_group->ChannelsMask.SetBit(channel_no);
                merge_group->ChannelsCount++;
                merge_group->ClipRect.Add(src_channel->_CmdBuffer[0].ClipRect);
                merge_group_mask |= (1 << merge_group_n);
            }

            // Invalidate current draw channel
            // (we don't clear DrawChannelFrozen/DrawChannelUnfrozen solely to facilitate debugging/later inspection of data)
            column->DrawChannelCurrent = (ImGuiTableDrawChannelIdx)-1;
        }

        // [DEBUG] Display merge groups
#if 0
        if (g.IO.KeyShift)
            for (int merge_group_n = 0; merge_group_n < IM_ARRAYSIZE(merge_groups); merge_group_n++)
            {
                MergeGroup* merge_group = &merge_groups[merge_group_n];
                if (merge_group->ChannelsCount == 0)
                    continue;
                char buf[32];
                ImFormatString(buf, 32, "MG%d:%d", merge_group_n, merge_group->ChannelsCount);
                ImVec2 text_pos = merge_group->ClipRect.Min + ImVec2(4, 4);
                ImVec2 text_size = CalcTextSize(buf, NULL);
                GetForegroundDrawList()->AddRectFilled(text_pos, text_pos + text_size, IM_COL32(0, 0, 0, 255));
                GetForegroundDrawList()->AddText(text_pos, IM_COL32(255, 255, 0, 255), buf, NULL);
                GetForegroundDrawList()->AddRect(merge_group->ClipRect.Min, merge_group->ClipRect.Max, IM_COL32(255, 255, 0, 255));
            }
#endif

        // 2. Rewrite channel list in our preferred order
        if (merge_group_mask != 0)
        {
            // We skip channel 0 (Bg0/Bg1) and 1 (Bg2 frozen) from the shuffling since they won't move - see channels allocation in TableSetupDrawChannels().
            const int LEADING_DRAW_CHANNELS = 2;
            g.DrawChannelsTempMergeBuffer.resize(splitter->_Count - LEADING_DRAW_CHANNELS); // Use shared temporary storage so the allocation gets amortized
            ImDrawChannel* dst_tmp = g.DrawChannelsTempMergeBuffer.Data;
            ImBitArray<IMGUIEX_TABLE_MAX_DRAW_CHANNELS> remaining_mask;                       // We need 132-bit of storage
            remaining_mask.ClearAllBits();
            remaining_mask.SetBitRange(LEADING_DRAW_CHANNELS, splitter->_Count);
            remaining_mask.ClearBit(table->Bg2DrawChannelUnfrozen);
            IM_ASSERT(has_freeze_v == false || table->Bg2DrawChannelUnfrozen != TABLE_DRAW_CHANNEL_BG2_FROZEN);
            int remaining_count = splitter->_Count - (has_freeze_v ? LEADING_DRAW_CHANNELS + 1 : LEADING_DRAW_CHANNELS);
            //ImRect host_rect = (table->InnerWindow == table->OuterWindow) ? table->InnerClipRect : table->HostClipRect;
            ImRect host_rect = table->HostClipRect;
            for (int merge_group_n = 0; merge_group_n < IM_ARRAYSIZE(merge_groups); merge_group_n++)
            {
                if (int merge_channels_count = merge_groups[merge_group_n].ChannelsCount)
                {
                    MergeGroup* merge_group = &merge_groups[merge_group_n];
                    ImRect merge_clip_rect = merge_group->ClipRect;

                    // Extend outer-most clip limits to match those of host, so draw calls can be merged even if
                    // outer-most columns have some outer padding offsetting them from their parent ClipRect.
                    // The principal cases this is dealing with are:
                    // - On a same-window table (not scrolling = single group), all fitting columns ClipRect -> will extend and match host ClipRect -> will merge
                    // - Columns can use padding and have left-most ClipRect.Min.x and right-most ClipRect.Max.x != from host ClipRect -> will extend and match host ClipRect -> will merge
                    // FIXME-TABLE FIXME-WORKRECT: We are wasting a merge opportunity on tables without scrolling if column doesn't fit
                    // within host clip rect, solely because of the half-padding difference between window->WorkRect and window->InnerClipRect.
                    if ((merge_group_n & 1) == 0 || !has_freeze_h)
                        merge_clip_rect.Min.x = ImMin(merge_clip_rect.Min.x, host_rect.Min.x);
                    if ((merge_group_n & 2) == 0 || !has_freeze_v)
                        merge_clip_rect.Min.y = ImMin(merge_clip_rect.Min.y, host_rect.Min.y);
                    if ((merge_group_n & 1) != 0)
                        merge_clip_rect.Max.x = ImMax(merge_clip_rect.Max.x, host_rect.Max.x);
                    if ((merge_group_n & 2) != 0 && (table->Flags & ImGuiTableFlags_NoHostExtendY) == 0)
                        merge_clip_rect.Max.y = ImMax(merge_clip_rect.Max.y, host_rect.Max.y);
#if 0
                    GetOverlayDrawList()->AddRect(merge_group->ClipRect.Min, merge_group->ClipRect.Max, IM_COL32(255, 0, 0, 200), 0.0f, ~0, 1.0f);
                    GetOverlayDrawList()->AddLine(merge_group->ClipRect.Min, merge_clip_rect.Min, IM_COL32(255, 100, 0, 200));
                    GetOverlayDrawList()->AddLine(merge_group->ClipRect.Max, merge_clip_rect.Max, IM_COL32(255, 100, 0, 200));
#endif
                    remaining_count -= merge_group->ChannelsCount;
                    for (int n = 0; n < IM_ARRAYSIZE(remaining_mask.Storage); n++)
                        remaining_mask.Storage[n] &= ~merge_group->ChannelsMask.Storage[n];
                    for (int n = 0; n < splitter->_Count && merge_channels_count != 0; n++)
                    {
                        // Copy + overwrite new clip rect
                        if (!merge_group->ChannelsMask.TestBit(n))
                            continue;
                        merge_group->ChannelsMask.ClearBit(n);
                        merge_channels_count--;

                        ImDrawChannel* channel = &splitter->_Channels[n];
                        IM_ASSERT(channel->_CmdBuffer.Size == 1 && merge_clip_rect.Contains(ImRect(channel->_CmdBuffer[0].ClipRect)));
                        channel->_CmdBuffer[0].ClipRect = merge_clip_rect.ToVec4();
                        memcpy(dst_tmp++, channel, sizeof(ImDrawChannel));
                    }
                }

                // Make sure Bg2DrawChannelUnfrozen appears in the middle of our groups (whereas Bg0/Bg1 and Bg2 frozen are fixed to 0 and 1)
                if (merge_group_n == 1 && has_freeze_v)
                    memcpy(dst_tmp++, &splitter->_Channels[table->Bg2DrawChannelUnfrozen], sizeof(ImDrawChannel));
            }

            // Append unmergeable channels that we didn't reorder at the end of the list
            for (int n = 0; n < splitter->_Count && remaining_count != 0; n++)
            {
                if (!remaining_mask.TestBit(n))
                    continue;
                ImDrawChannel* channel = &splitter->_Channels[n];
                memcpy(dst_tmp++, channel, sizeof(ImDrawChannel));
                remaining_count--;
            }
            IM_ASSERT(dst_tmp == g.DrawChannelsTempMergeBuffer.Data + g.DrawChannelsTempMergeBuffer.Size);
            memcpy(splitter->_Channels.Data + LEADING_DRAW_CHANNELS, g.DrawChannelsTempMergeBuffer.Data, (splitter->_Count - LEADING_DRAW_CHANNELS) * sizeof(ImDrawChannel));
        }
    }

    static void SetCurrentWindow(ImGuiWindow* window)
    {
        ImGuiContext& g = *GImGui;
        g.CurrentWindow = window;
        CurrentTable = window && window->DC.CurrentTableIdx != -1 ? Tables.GetByIndex(window->DC.CurrentTableIdx) : NULL;
        if (window)
            g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
    }

    void End()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;

        // Error checking: verify that user hasn't called End() too many times!
        if (g.CurrentWindowStack.Size <= 1 && g.WithinFrameScopeWithImplicitWindow)
        {
            IM_ASSERT_USER_ERROR(g.CurrentWindowStack.Size > 1, "Calling End() too many times!");
            return;
        }
        IM_ASSERT(g.CurrentWindowStack.Size > 0);

        // Error checking: verify that user doesn't directly call End() on a child window.
        if (window->Flags & ImGuiWindowFlags_ChildWindow)
            IM_ASSERT_USER_ERROR(g.WithinEndChild, "Must call EndChild() and not End()!");

        // Close anything that is open
        if (window->DC.CurrentColumns)
	        ImGui::EndColumns();
        ImGui::PopClipRect();   // Inner window clip rectangle

        // Stop logging
        if (!(window->Flags & ImGuiWindowFlags_ChildWindow))    // FIXME: add more options for scope of logging
	        ImGui::LogFinish();

        // Pop from window stack
        g.CurrentWindowStack.pop_back();
        if (window->Flags & ImGuiWindowFlags_Popup)
            g.BeginPopupStack.pop_back();
        window->DC.StackSizesOnBegin.CompareWithCurrentState();
        SetCurrentWindow(g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back());
    }

    void EndChild()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;

        IM_ASSERT(g.WithinEndChild == false);
        IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() calls

        g.WithinEndChild = true;
        if (window->BeginCount > 1)
        {
	        End();
        }
        else
        {
            ImVec2 sz = window->Size;
            if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
                sz.x = ImMax(4.0f, sz.x);
            if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y))
                sz.y = ImMax(4.0f, sz.y);
            End();

            ImGuiWindow* parent_window = g.CurrentWindow;
            ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
            ImGui::ItemSize(sz);
            if ((window->DC.NavLayerActiveMask != 0 || window->DC.NavHasScroll) && !(window->Flags & ImGuiWindowFlags_NavFlattened))
            {
	            ImGui::ItemAdd(bb, window->ChildId);
	            ImGui::RenderNavHighlight(bb, window->ChildId);

                // When browsing a window that has no activable items (scroll only) we keep a highlight on the child
                if (window->DC.NavLayerActiveMask == 0 && window == g.NavWindow)
	                ImGui::RenderNavHighlight(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavHighlightFlags_TypeThin);
            }
            else
            {
                // Not navigable into
                ImGui::ItemAdd(bb, 0);
            }
        }
        g.WithinEndChild = false;
    }

    static size_t TableSettingsCalcChunkSize(int columns_count)
    {
        return sizeof(ImGuiTableSettings) + (size_t)columns_count * sizeof(ImGuiTableColumnSettings);
    }

    // Clear and initialize empty settings instance
    static void TableSettingsInit(ImGuiTableSettings* settings, ImGuiID id, int columns_count, int columns_count_max)
    {
        IM_PLACEMENT_NEW(settings) ImGuiTableSettings();
        ImGuiTableColumnSettings* settings_column = settings->GetColumnSettings();
        for (int n = 0; n < columns_count_max; n++, settings_column++)
            IM_PLACEMENT_NEW(settings_column) ImGuiTableColumnSettings();
        settings->ID = id;
        settings->ColumnsCount = (ImGuiTableColumnIdx)columns_count;
        settings->ColumnsCountMax = (ImGuiTableColumnIdx)columns_count_max;
        settings->WantApply = true;
    }

    ImGuiTableSettings* TableSettingsCreate(ImGuiID id, int columns_count)
    {
        ImGuiTableSettings* settings = SettingsTables.alloc_chunk(TableSettingsCalcChunkSize(columns_count));
        TableSettingsInit(settings, id, columns_count, columns_count);
        return settings;
    }
	
    void TableSaveSettings(ImGuiTable* table)
    {
        table->IsSettingsDirty = false;
        if (table->Flags & ImGuiTableFlags_NoSavedSettings)
            return;

        // Bind or create settings data
        // ImGuiContext& g = *GImGui;
        ImGuiTableSettings* settings = TableGetBoundSettings(table);
        if (settings == nullptr)
        {
            settings = TableSettingsCreate(table->ID, table->ColumnsCount);
            table->SettingsOffset = SettingsTables.offset_from_ptr(settings);
        }
        settings->ColumnsCount = (ImGuiTableColumnIdx)table->ColumnsCount;

        // Serialize ImGuiTable/ImGuiTableColumn into ImGuiTableSettings/ImGuiTableColumnSettings
        IM_ASSERT(settings->ID == table->ID);
        IM_ASSERT(settings->ColumnsCount == table->ColumnsCount && settings->ColumnsCountMax >= settings->ColumnsCount);
        ImGuiTableColumn* column = table->Columns.Data;
        ImGuiTableColumnSettings* column_settings = settings->GetColumnSettings();

        bool save_ref_scale = false;
        settings->SaveFlags = ImGuiTableFlags_None;
        for (int n = 0; n < table->ColumnsCount; n++, column++, column_settings++)
        {
            const float width_or_weight = (column->Flags & ImGuiTableColumnFlags_WidthStretch) ? column->StretchWeight : column->WidthRequest;
            column_settings->WidthOrWeight = width_or_weight;
            column_settings->Index = (ImGuiTableColumnIdx)n;
            column_settings->DisplayOrder = column->DisplayOrder;
            column_settings->SortOrder = column->SortOrder;
            column_settings->SortDirection = column->SortDirection;
            column_settings->IsEnabled = column->IsEnabled;
            column_settings->IsStretch = (column->Flags & ImGuiTableColumnFlags_WidthStretch) ? 1 : 0;
            if ((column->Flags & ImGuiTableColumnFlags_WidthStretch) == 0)
                save_ref_scale = true;

            // We skip saving some data in the .ini file when they are unnecessary to restore our state.
            // Note that fixed width where initial width was derived from auto-fit will always be saved as InitStretchWeightOrWidth will be 0.0f.
            // FIXME-TABLE: We don't have logic to easily compare SortOrder to DefaultSortOrder yet so it's always saved when present.
            if (width_or_weight != column->InitStretchWeightOrWidth)
                settings->SaveFlags |= ImGuiTableFlags_Resizable;
            if (column->DisplayOrder != n)
                settings->SaveFlags |= ImGuiTableFlags_Reorderable;
            if (column->SortOrder != -1)
                settings->SaveFlags |= ImGuiTableFlags_Sortable;
            if (column->IsEnabled != ((column->Flags & ImGuiTableColumnFlags_DefaultHide) == 0))
                settings->SaveFlags |= ImGuiTableFlags_Hideable;
        }
        settings->SaveFlags &= table->Flags;
        settings->RefScale = save_ref_scale ? table->RefScale : 0.0f;

        ImGui::MarkIniSettingsDirty();
    }

    void EndTable()
    {
        ImGuiContext& g = *GImGui;
        ImGuiTable* table = CurrentTable;
        IM_ASSERT(table != NULL && "Only call EndTable() if BeginTable() returns true!");

        // This assert would be very useful to catch a common error... unfortunately it would probably trigger in some
        // cases, and for consistency user may sometimes output empty tables (and still benefit from e.g. outer border)
        //IM_ASSERT(table->IsLayoutLocked && "Table unused: never called TableNextRow(), is that the intent?");

        // If the user never got to call TableNextRow() or TableNextColumn(), we call layout ourselves to ensure all our
        // code paths are consistent (instead of just hoping that TableBegin/TableEnd will work), get borders drawn, etc.
        if (!table->IsLayoutLocked)
            TableUpdateLayout(table);

        const ImGuiTableFlags flags = table->Flags;
        ImGuiWindow* inner_window = table->InnerWindow;
        ImGuiWindow* outer_window = table->OuterWindow;
        IM_ASSERT(inner_window == g.CurrentWindow);
        IM_ASSERT(outer_window == inner_window || outer_window == inner_window->ParentWindow);

        if (table->IsInsideRow)
            TableEndRow(table);

        // Context menu in columns body
        // FIXME knox: This is not implemented, cause my plugins don't use it.
        // if (flags & ImGuiTableFlags_ContextMenuInBody)
        //     if (table->HoveredColumnBody != -1 && !ImGui::IsAnyItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        //         TableOpenContextMenu((int)table->HoveredColumnBody);

        // Finalize table height
        inner_window->DC.PrevLineSize = table->HostBackupPrevLineSize;
        inner_window->DC.CurrLineSize = table->HostBackupCurrLineSize;
        inner_window->DC.CursorMaxPos = table->HostBackupCursorMaxPos;
        const float inner_content_max_y = table->RowPosY2;
        IM_ASSERT(table->RowPosY2 == inner_window->DC.CursorPos.y);
        if (inner_window != outer_window)
            inner_window->DC.CursorMaxPos.y = inner_content_max_y;
        else if (!(flags & ImGuiTableFlags_NoHostExtendY))
            table->OuterRect.Max.y = table->InnerRect.Max.y = ImMax(table->OuterRect.Max.y, inner_content_max_y); // Patch OuterRect/InnerRect height
        table->WorkRect.Max.y = ImMax(table->WorkRect.Max.y, table->OuterRect.Max.y);
        table->LastOuterHeight = table->OuterRect.GetHeight();

        // Setup inner scrolling range
        // FIXME: This ideally should be done earlier, in BeginTable() SetNextWindowContentSize call, just like writing to inner_window->DC.CursorMaxPos.y,
        // but since the later is likely to be impossible to do we'd rather update both axises together.
        if (table->Flags & ImGuiTableFlags_ScrollX)
        {
            const float outer_padding_for_border = (table->Flags & ImGuiTableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
            float max_pos_x = table->InnerWindow->DC.CursorMaxPos.x;
            if (table->RightMostEnabledColumn != -1)
                max_pos_x = ImMax(max_pos_x, table->Columns[table->RightMostEnabledColumn].WorkMaxX + table->CellPaddingX + table->OuterPaddingX - outer_padding_for_border);
            if (table->ResizedColumn != -1)
                max_pos_x = ImMax(max_pos_x, table->ResizeLockMinContentsX2);
            table->InnerWindow->DC.CursorMaxPos.x = max_pos_x;
        }

        // Pop clipping rect
        if (!(flags & ImGuiTableFlags_NoClip))
            inner_window->DrawList->PopClipRect();
        inner_window->ClipRect = inner_window->DrawList->_ClipRectStack.back();

        // Draw borders
        if ((flags & ImGuiTableFlags_Borders) != 0)
            TableDrawBorders(table);

        // Flatten channels and merge draw calls
        table->DrawSplitter.SetCurrentChannel(inner_window->DrawList, 0);
        if ((table->Flags & ImGuiTableFlags_NoClip) == 0)
            TableMergeDrawChannels(table);
        table->DrawSplitter.Merge(inner_window->DrawList);

        // Update ColumnsAutoFitWidth to get us ahead for host using our size to auto-resize without waiting for next BeginTable()
        const float width_spacings = (table->OuterPaddingX * 2.0f) + (table->CellSpacingX1 + table->CellSpacingX2) * (table->ColumnsEnabledCount - 1);
        table->ColumnsAutoFitWidth = width_spacings + (table->CellPaddingX * 2.0f) * table->ColumnsEnabledCount;
        for (int column_n = 0; column_n < table->ColumnsCount; column_n++)
            if (table->EnabledMaskByIndex & (ImGuiTableColumnMask(1) << column_n))
            {
                ImGuiTableColumn* column = &table->Columns[column_n];
                if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && !(column->Flags & ImGuiTableColumnFlags_NoResize))
                    table->ColumnsAutoFitWidth += column->WidthRequest;
                else
                    table->ColumnsAutoFitWidth += TableGetColumnWidthAuto(table, column);
            }

        // Update scroll
        if ((table->Flags & ImGuiTableFlags_ScrollX) == 0 && inner_window != outer_window)
        {
            inner_window->Scroll.x = 0.0f;
        }
        else if (table->LastResizedColumn != -1 && table->ResizedColumn == -1 && inner_window->ScrollbarX && table->InstanceInteracted == table->InstanceCurrent)
        {
            // When releasing a column being resized, scroll to keep the resulting column in sight
            const float neighbor_width_to_keep_visible = table->MinColumnWidth + table->CellPaddingX * 2.0f;
            ImGuiTableColumn* column = &table->Columns[table->LastResizedColumn];
            if (column->MaxX < table->InnerClipRect.Min.x)
	            ImGui::SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x - neighbor_width_to_keep_visible, 1.0f);
            else if (column->MaxX > table->InnerClipRect.Max.x)
	            ImGui::SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x + neighbor_width_to_keep_visible, 1.0f);
        }

        // Apply resizing/dragging at the end of the frame
        if (table->ResizedColumn != -1 && table->InstanceCurrent == table->InstanceInteracted)
        {
            ImGuiTableColumn* column = &table->Columns[table->ResizedColumn];
            const float new_x2 = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + TABLE_RESIZE_SEPARATOR_HALF_THICKNESS);
            const float new_width = ImFloor(new_x2 - column->MinX - table->CellSpacingX1 - table->CellPaddingX * 2.0f);
            table->ResizedColumnNextWidth = new_width;
        }

        // Pop from id stack
        IM_ASSERT_USER_ERROR(inner_window->IDStack.back() == table->ID + table->InstanceCurrent, "Mismatching PushID/PopID!");
        IM_ASSERT_USER_ERROR(outer_window->DC.ItemWidthStack.Size >= table->HostBackupItemWidthStackSize, "Too many PopItemWidth!");
        ImGui::PopID();

        // Restore window data that we modified
        const ImVec2 backup_outer_max_pos = outer_window->DC.CursorMaxPos;
        inner_window->WorkRect = table->HostBackupWorkRect;
        inner_window->ParentWorkRect = table->HostBackupParentWorkRect;
        inner_window->SkipItems = table->HostSkipItems;
        outer_window->DC.CursorPos = table->OuterRect.Min;
        outer_window->DC.ItemWidth = table->HostBackupItemWidth;
        outer_window->DC.ItemWidthStack.Size = table->HostBackupItemWidthStackSize;
        outer_window->DC.ColumnsOffset = table->HostBackupColumnsOffset;

        // Layout in outer window
        // (FIXME: To allow auto-fit and allow desirable effect of SameLine() we dissociate 'used' vs 'ideal' size by overriding
        // CursorPosPrevLine and CursorMaxPos manually. That should be a more general layout feature, see same problem e.g. #3414)
        if (inner_window != outer_window)
        {
	        EndChild();
        }
        else
        {
	        ImGui::ItemSize(table->OuterRect.GetSize());
	        ImGui::ItemAdd(table->OuterRect, 0);
        }

        // Override declared contents width/height to enable auto-resize while not needlessly adding a scrollbar
        if (table->Flags & ImGuiTableFlags_NoHostExtendX)
        {
            // FIXME-TABLE: Could we remove this section?
            // ColumnsAutoFitWidth may be one frame ahead here since for Fixed+NoResize is calculated from latest contents
            IM_ASSERT((table->Flags & ImGuiTableFlags_ScrollX) == 0);
            outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, table->OuterRect.Min.x + table->ColumnsAutoFitWidth);
        }
        else if (table->UserOuterSize.x <= 0.0f)
        {
            const float decoration_size = (table->Flags & ImGuiTableFlags_ScrollX) ? inner_window->ScrollbarSizes.x : 0.0f;
            outer_window->DC.IdealMaxPos.x = ImMax(outer_window->DC.IdealMaxPos.x, table->OuterRect.Min.x + table->ColumnsAutoFitWidth + decoration_size - table->UserOuterSize.x);
            outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, ImMin(table->OuterRect.Max.x, table->OuterRect.Min.x + table->ColumnsAutoFitWidth));
        }
        else
        {
            outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, table->OuterRect.Max.x);
        }
        if (table->UserOuterSize.y <= 0.0f)
        {
            const float decoration_size = (table->Flags & ImGuiTableFlags_ScrollY) ? inner_window->ScrollbarSizes.y : 0.0f;
            outer_window->DC.IdealMaxPos.y = ImMax(outer_window->DC.IdealMaxPos.y, inner_content_max_y + decoration_size - table->UserOuterSize.y);
            outer_window->DC.CursorMaxPos.y = ImMax(backup_outer_max_pos.y, ImMin(table->OuterRect.Max.y, inner_content_max_y));
        }
        else
        {
            // OuterRect.Max.y may already have been pushed downward from the initial value (unless ImGuiTableFlags_NoHostExtendY is set)
            outer_window->DC.CursorMaxPos.y = ImMax(backup_outer_max_pos.y, table->OuterRect.Max.y);
        }

        // Save settings
        if (table->IsSettingsDirty)
            TableSaveSettings(table);
        table->IsInitializing = false;

        // Clear or restore current table, if any
        IM_ASSERT(g.CurrentWindow == outer_window && CurrentTable == table);
        CurrentTableStack.pop_back();
        CurrentTable = CurrentTableStack.Size ? Tables.GetByIndex(CurrentTableStack.back().Index) : NULL;
        outer_window->DC.CurrentTableIdx = CurrentTable ? Tables.GetIndex(CurrentTable) : -1;
    }

	const char* TableGetColumnName(int column_n)
	{
	    ImGuiTable* table = CurrentTable;
	    if (!table)
	        return NULL;
	    if (column_n < 0)
	        column_n = table->CurrentColumn;
	    return TableGetColumnName(table, column_n);
	}

    const char* TableGetColumnName(const ImGuiTable* table, int column_n)
    {
        if (table->IsLayoutLocked == false && column_n >= table->DeclColumnsCount)
            return ""; // NameOffset is invalid at this point
        const ImGuiTableColumn* column = &table->Columns[column_n];
        if (column->NameOffset == -1)
            return "";
        return &table->ColumnsNames.Buf[column->NameOffset];
    }


    // Note that the NoSortAscending/NoSortDescending flags are processed in TableSortSpecsSanitize(), and they may change/revert
    // the value of SortDirection. We could technically also do it here but it would be unnecessary and duplicate code.
    void TableSetColumnSortDirection(int column_n, ImGuiSortDirection sort_direction, bool append_to_sort_specs)
    {
        ImGuiTable* table = CurrentTable;

        if (!(table->Flags & ImGuiTableFlags_SortMulti))
            append_to_sort_specs = false;
        if (!(table->Flags & ImGuiTableFlags_SortTristate))
            IM_ASSERT(sort_direction != ImGuiSortDirection_None);

        ImGuiTableColumnIdx sort_order_max = 0;
        if (append_to_sort_specs)
            for (int other_column_n = 0; other_column_n < table->ColumnsCount; other_column_n++)
                sort_order_max = ImMax(sort_order_max, table->Columns[other_column_n].SortOrder);

        ImGuiTableColumn* column = &table->Columns[column_n];
        column->SortDirection = (ImU8)sort_direction;
        if (column->SortDirection == ImGuiSortDirection_None)
            column->SortOrder = -1;
        else if (column->SortOrder == -1 || !append_to_sort_specs)
            column->SortOrder = append_to_sort_specs ? sort_order_max + 1 : 0;

        for (int other_column_n = 0; other_column_n < table->ColumnsCount; other_column_n++)
        {
            ImGuiTableColumn* other_column = &table->Columns[other_column_n];
            if (other_column != column && !append_to_sort_specs)
                other_column->SortOrder = -1;
            TableFixColumnSortDirection(table, other_column);
        }
        table->IsSettingsDirty = true;
        table->IsSortSpecsDirty = true;
    }
	
    void SetTooltip(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        ImGui::BeginTooltipEx(0, ImGuiTooltipFlags_OverridePreviousTooltip);
        ImGui::TextV(fmt, args);
        // ImGui::EndTooltip();
        IM_ASSERT(ImGui::GetCurrentWindowRead()->Flags & ImGuiWindowFlags_Tooltip);   // Mismatched BeginTooltip()/EndTooltip() calls
        End();
        va_end(args);
    }

    // This is a copy of `ImGui::TableHeader(const char* label)`
    // I removed the line, where the header is printed, so i can use it with image only headers.
    // When "show_label" is true, the label will be printed, as in the default one.
    //
    // Emit a column header (text + optional sort order)
    // We cpu-clip text here so that all columns headers can be merged into a same draw call.
    // Note that because of how we cpu-clip and display sorting indicators, you _cannot_ use SameLine() after a TableHeader()
    void TableHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment) {
        // TODO change eventually
        const float image_size = 16.f;

    	// Show label, if texture is not loaded
    	if (!texture) {
            show_label = true;
    	}

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return;

        ImGuiTable* table = CurrentTable;
        IM_ASSERT(table != NULL && "Need to call TableHeader() after BeginTable()!");
        IM_ASSERT(table->CurrentColumn != -1);
        const int column_n = table->CurrentColumn;
        ImGuiTableColumn* column = &table->Columns[column_n];

        // Label
        if (label == NULL)
            label = "";
        const char* label_end = ImGui::FindRenderedTextEnd(label);
        ImVec2 label_size = ImGui::CalcTextSize(label, label_end, true);
        ImVec2 label_pos = window->DC.CursorPos;

        // If we already got a row height, there's use that.
        // FIXME-TABLE: Padding problem if the correct outer-padding CellBgRect strays off our ClipRect?
        ImRect cell_r = TableGetCellBgRect(table, column_n);
        float label_height = table->RowMinHeight - table->CellPaddingY * 2.0f;
        if (show_label) {
            label_height = ImMax(label_size.y, label_height);
        }
        else {
            label_height = ImMax(image_size, label_height);
        }

        // Calculate ideal size for sort order arrow
        float w_arrow = 0.0f;
        float w_sort_text = 0.0f;
        char sort_order_suf[4] = "";
        const float ARROW_SCALE = 0.65f;
        if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
            w_arrow = ImFloor(g.FontSize * ARROW_SCALE + g.Style.FramePadding.x);
            if (column->SortOrder > 0) {
                ImFormatString(sort_order_suf, IM_ARRAYSIZE(sort_order_suf), "%d", column->SortOrder + 1);
                w_sort_text = g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(sort_order_suf).x;
            }
        }

        // We feed our unclipped width to the column without writing on CursorMaxPos, so that column is still considering for merging.
        float max_pos_x = label_pos.x + w_sort_text + w_arrow;
        if (show_label) {
            max_pos_x += label_size.x;
        }
        else {
            max_pos_x += image_size;
        }
        column->ContentMaxXHeadersUsed = ImMax(column->ContentMaxXHeadersUsed, column->WorkMaxX);
        column->ContentMaxXHeadersIdeal = ImMax(column->ContentMaxXHeadersIdeal, max_pos_x);

        // Keep header highlighted when context menu is open.
        const bool selected = (table->IsContextPopupOpen && table->ContextPopupColumn == column_n && table->InstanceInteracted == table->InstanceCurrent);
        ImGuiID id = window->GetID(label);
        ImRect bb(cell_r.Min.x, cell_r.Min.y, cell_r.Max.x, ImMax(cell_r.Max.y, cell_r.Min.y + label_height + g.Style.CellPadding.y * 2.0f));
        ImGui::ItemSize(ImVec2(0.0f, label_height)); // Don't declare unclipped width, it'll be fed ContentMaxPosHeadersIdeal
        if (!ImGui::ItemAdd(bb, id))
            return;

        //GetForegroundDrawList()->AddRect(cell_r.Min, cell_r.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]
        //GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]

        // Using AllowItemOverlap mode because we cover the whole cell, and we want user to be able to submit subsequent items.
        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_AllowItemOverlap);
        if (g.ActiveId != id)
            ImGui::SetItemAllowOverlap();
        if (held || hovered || selected) {
            const ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            //RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
            TableSetBgColor(ImGuiTableBgTarget_CellBg, col, table->CurrentColumn);
            ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
        }
        else {
            // Submit single cell bg color in the case we didn't submit a full header row
            if ((table->RowFlags & ImGuiTableRowFlags_Headers) == 0)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg), table->CurrentColumn);
        }
        if (held)
            table->HeldHeaderColumn = (ImGuiTableColumnIdx)column_n;
        window->DC.CursorPos.y -= g.Style.ItemSpacing.y * 0.5f;

        // Drag and drop to re-order columns.
        // FIXME-TABLE: Scroll request while reordering a column and it lands out of the scrolling zone.
        if (held && (table->Flags & ImGuiTableFlags_Reorderable) && ImGui::IsMouseDragging(0) && !g.DragDropActive) {
            // While moving a column it will jump on the other side of the mouse, so we also test for MouseDelta.x
            table->ReorderColumn = (ImGuiTableColumnIdx)column_n;
            table->InstanceInteracted = table->InstanceCurrent;

            // We don't reorder: through the frozen<>unfrozen line, or through a column that is marked with ImGuiTableColumnFlags_NoReorder.
            if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < cell_r.Min.x)
                if (ImGuiTableColumn* prev_column = (column->PrevEnabledColumn != -1) ? &table->Columns[column->PrevEnabledColumn] : NULL)
                    if (!((column->Flags | prev_column->Flags) & ImGuiTableColumnFlags_NoReorder))
                        if ((column->IndexWithinEnabledSet < table->FreezeColumnsRequest) == (prev_column->IndexWithinEnabledSet < table->FreezeColumnsRequest))
                            table->ReorderColumnDir = -1;
            if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > cell_r.Max.x)
                if (ImGuiTableColumn* next_column = (column->NextEnabledColumn != -1) ? &table->Columns[column->NextEnabledColumn] : NULL)
                    if (!((column->Flags | next_column->Flags) & ImGuiTableColumnFlags_NoReorder))
                        if ((column->IndexWithinEnabledSet < table->FreezeColumnsRequest) == (next_column->IndexWithinEnabledSet < table->FreezeColumnsRequest))
                            table->ReorderColumnDir = +1;
        }

        // Sort order arrow
        const float ellipsis_max = cell_r.Max.x - w_arrow - w_sort_text;
        if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
            if (column->SortOrder != -1) {
                float x = ImMax(cell_r.Min.x, cell_r.Max.x - w_arrow - w_sort_text);
                float y = label_pos.y;
                if (column->SortOrder > 0) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_Text, 0.70f));
                    ImGui::RenderText(ImVec2(x + g.Style.ItemInnerSpacing.x, y), sort_order_suf);
                    ImGui::PopStyleColor();
                    x += w_sort_text;
                }
                ImGui::RenderArrow(window->DrawList, ImVec2(x, y), ImGui::GetColorU32(ImGuiCol_Text),
                    column->SortDirection == ImGuiSortDirection_Ascending ? ImGuiDir_Up : ImGuiDir_Down, ARROW_SCALE);
            }

            // Handle clicking on column header to adjust Sort Order
            if (pressed && table->ReorderColumn != column_n) {
                ImGuiSortDirection sort_direction = TableGetColumnNextSortDirection(column);
                TableSetColumnSortDirection(column_n, sort_direction, g.IO.KeyShift);
            }
        }

        // Render clipped label. Clipping here ensure that in the majority of situations, all our header cells will
        // be merged into a single draw call.
        //window->DrawList->AddCircleFilled(ImVec2(ellipsis_max, label_pos.y), 40, IM_COL32_WHITE);
        if (show_label) {
            // ImGui::RenderTextEllipsis(window->DrawList, label_pos, ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max,
            // 	ellipsis_max, label, label_end, &label_size);

            float newX = label_pos.x;

            switch (alignment) {
            case Alignment::Center:
                // newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (label_size.x / 2);
                newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (label_size.x / 2);
                // ImGui::SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
                break;
            case Alignment::Right:
                newX = ellipsis_max - label_size.x;
                // ImGui::SetCursorPosX(cursorPosX + textSpace - contentSize.x);
                break;
            default: [[fallthrough]];
            }

            ImGui::RenderTextEllipsis(window->DrawList, ImVec2(newX, label_pos.y), ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max,
                ellipsis_max, label, label_end, &label_size);
        }
        else {
            float newX = label_pos.x;

            switch (alignment) {
            case Alignment::Center:
                // newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (image_size / 2);
                newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (image_size / 2);
                // ImGui::SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
                break;
            case Alignment::Right:
                newX = ellipsis_max - image_size;
                // ImGui::SetCursorPosX(cursorPosX + textSpace - contentSize.x);
                break;
            }

            ImRect ibb(ImVec2(newX, label_pos.y), ImVec2(newX, label_pos.y) + image_size);

            window->DrawList->AddImage(texture, ibb.Min, ibb.Max);
        }

        // const bool text_clipped = label_size.x > (ellipsis_max - label_pos.x);
        // if (text_clipped && hovered && g.HoveredIdNotActiveTimer > g.TooltipSlowDelay)
            // ImGui::SetTooltip("%.*s", (int)(label_end - label), label);
        if (ImGui::IsItemHovered()) {
            SetTooltip("%s", label);
        }

        // We don't use BeginPopupContextItem() because we want the popup to stay up even after the column is hidden
        // if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered())
            // ImGui::TableOpenContextMenu(column_n);
    }

    void MenuItemTableColumnVisibility(ImGuiTable* table, int columnIdx) {
        ImGuiTableColumn& column = table->Columns[columnIdx];
        const char* columnName = TableGetColumnName(table, columnIdx);
        // Make sure we can't hide the last active column
        bool menu_item_active = (column.Flags & ImGuiTableColumnFlags_NoHide) ? false : true;
        if (column.IsEnabled && table->ColumnsEnabledCount <= 1)
            menu_item_active = false;
        if (ImGui::MenuItem(columnName, NULL, column.IsEnabled, menu_item_active))
            column.IsEnabledNextFrame = !column.IsEnabled;
    }

    // Calculate next sort direction that would be set after clicking the column
	// - If the PreferSortDescending flag is set, we will default to a Descending direction on the first click.
	// - Note that the PreferSortAscending flag is never checked, it is essentially the default and therefore a no-op.
    IM_STATIC_ASSERT(ImGuiSortDirection_None == 0 && ImGuiSortDirection_Ascending == 1 && ImGuiSortDirection_Descending == 2);
    ImGuiSortDirection TableGetColumnNextSortDirection(ImGuiTableColumn* column)
    {
        IM_ASSERT(column->SortDirectionsAvailCount > 0);
        if (column->SortOrder == -1)
            return TableGetColumnAvailSortDirection(column, 0);
        for (int n = 0; n < 3; n++)
            if (column->SortDirection == TableGetColumnAvailSortDirection(column, n))
                return TableGetColumnAvailSortDirection(column, (n + 1) % column->SortDirectionsAvailCount);
        IM_ASSERT(0);
        return ImGuiSortDirection_None;
    }

    // [Internal] Absolute coordinate. Saner. This is not exposed until we finishing refactoring work rect features.
    ImVec2 GetContentRegionMaxAbs()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImVec2 mx = window->ContentRegionRect.Max;
        if (window->DC.CurrentColumns || CurrentTable)
            mx.x = window->WorkRect.Max.x;
        return mx;
    }

    ImVec2 GetContentRegionAvail()
    {
        ImGuiWindow* window = GImGui->CurrentWindow;
        return GetContentRegionMaxAbs() - window->DC.CursorPos;
    }

    float GetColumnWidth(int column_index)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiOldColumns* columns = window->DC.CurrentColumns;
        if (columns == nullptr)
            return GetContentRegionAvail().x;

        if (column_index < 0)
            column_index = columns->Current;
        return ImGui::GetColumnOffsetFromNorm(columns, columns->Columns[column_index + 1].OffsetNorm - columns->Columns[column_index].OffsetNorm);
    }

    void AlignedTextColumn(Alignment alignment, const char* text, ...) {
        va_list args;
        va_start(args, text);
        char buf[4096];
        ImFormatStringV(buf, 4096, text, args);
        va_end(args);

        const float posX = ImGui::GetCursorPosX();
        float newX = posX;
        float textWidth = ImGui::CalcTextSize(buf).x;
        float columnWidth = GetColumnWidth();

        switch (alignment) {
        case Alignment::Left:
            break;
        case Alignment::Center:
            newX = posX + columnWidth / 2 - textWidth / 2;
            break;
        case Alignment::Right:
            newX = posX + columnWidth - textWidth;
            break;
        }

        // Clip to left, if text is bigger than current column
        if (newX < posX) {
            newX = posX;
        }

        ImGui::SetCursorPosX(newX);

        ImGui::TextUnformatted(buf);
    }

    static void TableSettingsHandler_ClearAll(ImGuiContext*, ImGuiSettingsHandler*)
    {
        for (int i = 0; i != Tables.GetSize(); i++)
            Tables.GetByIndex(i)->SettingsOffset = -1;
        SettingsTables.clear();
    }

    static void* TableSettingsHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
    {
        ImGuiID id = 0;
        int columns_count = 0;
        if (sscanf(name, "0x%08X,%d", &id, &columns_count) < 2)
            return NULL;

        if (ImGuiTableSettings* settings = TableSettingsFindByID(id))
        {
            if (settings->ColumnsCountMax >= columns_count)
            {
                TableSettingsInit(settings, id, columns_count, settings->ColumnsCountMax); // Recycle
                return settings;
            }
            settings->ID = 0; // Invalidate storage, we won't fit because of a count change
        }
        return TableSettingsCreate(id, columns_count);
    }

    static void TableSettingsHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
    {
        // "Column 0  UserID=0x42AD2D21 Width=100 Visible=1 Order=0 Sort=0v"
        ImGuiTableSettings* settings = (ImGuiTableSettings*)entry;
        float f = 0.0f;
        int column_n = 0, r = 0, n = 0;

        if (sscanf(line, "RefScale=%f", &f) == 1) { settings->RefScale = f; return; }

        if (sscanf(line, "Column %d%n", &column_n, &r) == 1)
        {
            if (column_n < 0 || column_n >= settings->ColumnsCount)
                return;
            line = ImStrSkipBlank(line + r);
            char c = 0;
            ImGuiTableColumnSettings* column = settings->GetColumnSettings() + column_n;
            column->Index = (ImGuiTableColumnIdx)column_n;
            if (sscanf(line, "UserID=0x%08X%n", (ImU32*)&n, &r) == 1) { line = ImStrSkipBlank(line + r); column->UserID = (ImGuiID)n; }
            if (sscanf(line, "Width=%d%n", &n, &r) == 1) { line = ImStrSkipBlank(line + r); column->WidthOrWeight = (float)n; column->IsStretch = 0; settings->SaveFlags |= ImGuiTableFlags_Resizable; }
            if (sscanf(line, "Weight=%f%n", &f, &r) == 1) { line = ImStrSkipBlank(line + r); column->WidthOrWeight = f; column->IsStretch = 1; settings->SaveFlags |= ImGuiTableFlags_Resizable; }
            if (sscanf(line, "Visible=%d%n", &n, &r) == 1) { line = ImStrSkipBlank(line + r); column->IsEnabled = (ImU8)n; settings->SaveFlags |= ImGuiTableFlags_Hideable; }
            if (sscanf(line, "Order=%d%n", &n, &r) == 1) { line = ImStrSkipBlank(line + r); column->DisplayOrder = (ImGuiTableColumnIdx)n; settings->SaveFlags |= ImGuiTableFlags_Reorderable; }
            if (sscanf(line, "Sort=%d%c%n", &n, &c, &r) == 2) { line = ImStrSkipBlank(line + r); column->SortOrder = (ImGuiTableColumnIdx)n; column->SortDirection = (c == '^') ? ImGuiSortDirection_Descending : ImGuiSortDirection_Ascending; settings->SaveFlags |= ImGuiTableFlags_Sortable; }
        }
    }

    // Apply to existing windows (if any)
    static void TableSettingsHandler_ApplyAll(ImGuiContext*, ImGuiSettingsHandler*)
    {
        for (int i = 0; i != Tables.GetSize(); i++)
        {
            ImGuiTable* table = Tables.GetByIndex(i);
            table->IsSettingsRequestLoad = true;
            table->SettingsOffset = -1;
        }
    }

    static void TableSettingsHandler_WriteAll(ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        for (ImGuiTableSettings* settings = SettingsTables.begin(); settings != NULL; settings = SettingsTables.next_chunk(settings))
        {
            if (settings->ID == 0) // Skip ditched settings
                continue;

            // TableSaveSettings() may clear some of those flags when we establish that the data can be stripped
            // (e.g. Order was unchanged)
            const bool save_size = (settings->SaveFlags & ImGuiTableFlags_Resizable) != 0;
            const bool save_visible = (settings->SaveFlags & ImGuiTableFlags_Hideable) != 0;
            const bool save_order = (settings->SaveFlags & ImGuiTableFlags_Reorderable) != 0;
            const bool save_sort = (settings->SaveFlags & ImGuiTableFlags_Sortable) != 0;
            if (!save_size && !save_visible && !save_order && !save_sort)
                continue;

            buf->reserve(buf->size() + 30 + settings->ColumnsCount * 50); // ballpark reserve
            buf->appendf("[%s][0x%08X,%d]\n", handler->TypeName, settings->ID, settings->ColumnsCount);
            if (settings->RefScale != 0.0f)
                buf->appendf("RefScale=%g\n", settings->RefScale);
            ImGuiTableColumnSettings* column = settings->GetColumnSettings();
            for (int column_n = 0; column_n < settings->ColumnsCount; column_n++, column++)
            {
                // "Column 0  UserID=0x42AD2D21 Width=100 Visible=1 Order=0 Sort=0v"
                buf->appendf("Column %-2d", column_n);
                if (column->UserID != 0)                    buf->appendf(" UserID=%08X", column->UserID);
                if (save_size && column->IsStretch)         buf->appendf(" Weight=%.4f", column->WidthOrWeight);
                if (save_size && !column->IsStretch)        buf->appendf(" Width=%d", (int)column->WidthOrWeight);
                if (save_visible)                           buf->appendf(" Visible=%d", column->IsEnabled);
                if (save_order)                             buf->appendf(" Order=%d", column->DisplayOrder);
                if (save_sort && column->SortOrder != -1)   buf->appendf(" Sort=%d%c", column->SortOrder, (column->SortDirection == ImGuiSortDirection_Ascending) ? 'v' : '^');
                buf->append("\n");
            }
            buf->append("\n");
        }
    }

	void RegisterSettingsHandler(const char* name) {
        ImGuiContext* context = ImGui::GetCurrentContext();
        ImGuiSettingsHandler ini_handler;
        ini_handler.TypeName = name;
        ini_handler.TypeHash = ImHashStr(name);
        ini_handler.ClearAllFn = TableSettingsHandler_ClearAll;
        ini_handler.ReadOpenFn = TableSettingsHandler_ReadOpen;
        ini_handler.ReadLineFn = TableSettingsHandler_ReadLine;
        ini_handler.ApplyAllFn = TableSettingsHandler_ApplyAll;
        ini_handler.WriteAllFn = TableSettingsHandler_WriteAll;
        context->SettingsHandlers.push_back(ini_handler);
    }

    int TableGetColumnIndex()
    {
        ImGuiTable* table = CurrentTable;
        if (!table)
            return 0;
        return table->CurrentColumn;
    }

	// We allow querying for an extra column in order to poll the IsHovered state of the right-most section
	ImGuiTableColumnFlags TableGetColumnFlags(int column_n)
	{
	    ImGuiTable* table = CurrentTable;
	    if (!table)
	        return ImGuiTableColumnFlags_None;
	    if (column_n < 0)
	        column_n = table->CurrentColumn;
	    if (column_n == table->ColumnsCount)
	        return (table->HoveredColumnBody == column_n) ? ImGuiTableColumnFlags_IsHovered : ImGuiTableColumnFlags_None;
	    return table->Columns[column_n].Flags;
	}

	float TableGetHeaderRowHeight()
	{
	    // Caring for a minor edge case:
	    // Calculate row height, for the unlikely case that some labels may be taller than others.
	    // If we didn't do that, uneven header height would highlight but smaller one before the tallest wouldn't catch input for all height.
	    // In your custom header row you may omit this all together and just call TableNextRow() without a height...
	    float row_height = ImGui::GetTextLineHeight();
	    int columns_count = ImGui::TableGetColumnCount();
	    for (int column_n = 0; column_n < columns_count; column_n++)
	        if (TableGetColumnFlags(column_n) & ImGuiTableColumnFlags_IsEnabled)
	            row_height = ImMax(row_height, ImGui::CalcTextSize(TableGetColumnName(column_n)).y);
	    row_height += ImGui::GetStyle().CellPadding.y * 2.0f;
	    return row_height;
	}

	void TableSetupScrollFreeze(int columns, int rows)
	{
	    ImGuiTable* table = CurrentTable;
	    IM_ASSERT(table != NULL && "Need to call TableSetupColumn() after BeginTable()!");
	    IM_ASSERT(table->IsLayoutLocked == false && "Need to call TableSetupColumn() before first row!");
	    IM_ASSERT(columns >= 0 && columns < IMGUI_TABLE_MAX_COLUMNS);
	    IM_ASSERT(rows >= 0 && rows < 128); // Arbitrary limit

	    table->FreezeColumnsRequest = (table->Flags & ImGuiTableFlags_ScrollX) ? (ImGuiTableColumnIdx)columns : 0;
	    table->FreezeColumnsCount = (table->InnerWindow->Scroll.x != 0.0f) ? table->FreezeColumnsRequest : 0;
	    table->FreezeRowsRequest = (table->Flags & ImGuiTableFlags_ScrollY) ? (ImGuiTableColumnIdx)rows : 0;
	    table->FreezeRowsCount = (table->InnerWindow->Scroll.y != 0.0f) ? table->FreezeRowsRequest : 0;
	    table->IsUnfrozenRows = (table->FreezeRowsCount == 0); // Make sure this is set before TableUpdateLayout() so ImGuiListClipper can benefit from it.b
	}
}

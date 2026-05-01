#pragma once

#include <compare>
#include <cstdint>
#include <limits>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ArcdpsExtension/arcdps_structs.h>

#define IM_COL32_DISABLE                IM_COL32(0,0,0,1)    // Special sentinel code which cannot be used as a regular color.
#define IMGUI_TABLE_MAX_COLUMNS         512                 // Arbitrary "safety" maximum, may be lifted in the future if needed. Must fit in ImGuiTableColumnIdx/ImGuiTableDrawChannelIdx.

/**
 * This implementation is mostly copied over code from ImGui!
 *
 * USAGE:
 * - Call `RegisterSettingsHandler(const char* name)` in `mod_init()` to enable saving settings in imgui.ini
 * - Call `BeginTable()` to start and `EndTable()` to end the bigger table.
 * When you use BigTable you have to use these functions, instead of the original ImGui ones, inside your Table.
 * This BigTable implementation uses it's own space/structs/functions/storage and is mostly independent from ImGui itself.
 * Everything is still allocated in the given ImGui Context.
 * The current maximal amount of columns is raised to 128.
 * To heighten the limit update the macro above and make an ImU256 implementation. Don't forget to update the typedef `ImGuiTableColumnMask`.
 *
 * Do NOT mix-up the original ImGuiTable implementation and this one, they are incompatible with each other.
 * 
 */
namespace ImGuiEx::BigTable {
    typedef ImS16 ImGuiTableColumnIdx;
    typedef ImU16 ImGuiTableDrawChannelIdx;

    // [Internal] sizeof() ~ 112
    // We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
    // We use the terminology "Clipped" to refer to a column that is out of sight because of scrolling/clipping.
    // This is in contrast with some user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible" to mean "not clipped".
    struct ImGuiTableColumn
    {
        ImGuiTableColumnFlags   Flags;                          // Flags after some patching (not directly same as provided by user). See ImGuiTableColumnFlags_
        float                   WidthGiven;                     // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout(). May be > WidthRequest to honor minimum width, may be < WidthRequest to honor shrinking columns down in tight space.
        float                   MinX;                           // Absolute positions
        float                   MaxX;
        float                   WidthRequest;                   // Master width absolute value when !(Flags & _WidthStretch). When Stretch this is derived every frame from StretchWeight in TableUpdateLayout()
        float                   WidthAuto;                      // Automatic width
        float                   WidthMax;                       // Maximum width (FIXME: overwritten by each instance)
        float                   StretchWeight;                  // Master width weight when (Flags & _WidthStretch). Often around ~1.0f initially.
        float                   InitStretchWeightOrWidth;       // Value passed to TableSetupColumn(). For Width it is a content width (_without padding_).
        ImRect                  ClipRect;                       // Clipping rectangle for the column
        ImGuiID                 UserID;                         // Optional, value passed to TableSetupColumn()
        float                   WorkMinX;                       // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start position when entering column
        float                   WorkMaxX;                       // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
        float                   ItemWidth;                      // Current item width for the column, preserved across rows
        float                   ContentMaxXFrozen;              // Contents maximum position for frozen rows (apart from headers), from which we can infer content width.
        float                   ContentMaxXUnfrozen;
        float                   ContentMaxXHeadersUsed;         // Contents maximum position for headers rows (regardless of freezing). TableHeader() automatically softclip itself + report ideal desired size, to avoid creating extraneous draw calls
        float                   ContentMaxXHeadersIdeal;
        ImS16                   NameOffset;                     // Offset into parent ColumnsNames[]
        ImGuiTableColumnIdx     DisplayOrder;                   // Index within Table's IndexToDisplayOrder[] (column may be reordered by users)
        ImGuiTableColumnIdx     IndexWithinEnabledSet;          // Index within enabled/visible set (<= IndexToDisplayOrder)
        ImGuiTableColumnIdx     PrevEnabledColumn;              // Index of prev enabled/visible column within Columns[], -1 if first enabled/visible column
        ImGuiTableColumnIdx     NextEnabledColumn;              // Index of next enabled/visible column within Columns[], -1 if last enabled/visible column
        ImGuiTableColumnIdx     SortOrder;                      // Index of this column within sort specs, -1 if not sorting on this column, 0 for single-sort, may be >0 on multi-sort
        ImGuiTableDrawChannelIdx DrawChannelCurrent;            // Index within DrawSplitter.Channels[]
        ImGuiTableDrawChannelIdx DrawChannelFrozen;             // Draw channels for frozen rows (often headers)
        ImGuiTableDrawChannelIdx DrawChannelUnfrozen;           // Draw channels for unfrozen rows
        bool                    IsEnabled;                      // IsUserEnabled && (Flags & ImGuiTableColumnFlags_Disabled) == 0
        bool                    IsUserEnabled;                  // Is the column not marked Hidden by the user? (unrelated to being off view, e.g. clipped by scrolling).
        bool                    IsUserEnabledNextFrame;
        bool                    IsVisibleX;                     // Is actually in view (e.g. overlapping the host window clipping rectangle, not scrolled).
        bool                    IsVisibleY;
        bool                    IsRequestOutput;                // Return value for TableSetColumnIndex() / TableNextColumn(): whether we request user to output contents or not.
        bool                    IsSkipItems;                    // Do we want item submissions to this column to be completely ignored (no layout will happen).
        bool                    IsPreserveWidthAuto;
        ImS8                    NavLayerCurrent;                // ImGuiNavLayer in 1 byte
        ImU8                    AutoFitQueue;                   // Queue of 8 values for the next 8 frames to request auto-fit
        ImU8                    CannotSkipItemsQueue;           // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
        ImU8                    SortDirection : 2;              // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending
        ImU8                    SortDirectionsAvailCount : 2;   // Number of available sort directions (0 to 3)
        ImU8                    SortDirectionsAvailMask : 4;    // Mask of available sort directions (1-bit each)
        ImU8                    SortDirectionsAvailList;        // Ordered list of available sort directions (2-bits each, total 8-bits)

        ImGuiTableColumn()
        {
            memset((void*)this, 0, sizeof(*this));
            StretchWeight = WidthRequest = -1.0f;
            NameOffset = -1;
            DisplayOrder = IndexWithinEnabledSet = -1;
            PrevEnabledColumn = NextEnabledColumn = -1;
            SortOrder = -1;
            SortDirection = ImGuiSortDirection_None;
            DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (ImU8)-1;
        }
    };

    // Transient cell data stored per row.
    // sizeof() ~ 6 bytes
    struct ImGuiTableCellData
    {
        ImU32                       BgColor;    // Actual color
        ImGuiTableColumnIdx         Column;     // Column number
    };

    // sizeof() ~ 592 bytes + heap allocs described in TableBeginInitMemory()
    struct IMGUI_API ImGuiTable
    {
        ImGuiID                     ID;
        ImGuiTableFlags             Flags;
        void* RawData;                    // Single allocation to hold Columns[], DisplayOrderToIndex[], and RowCellData[]
        ImGuiTableTempData* TempData;                   // Transient data while table is active. Point within g.CurrentTableStack[]
        ImSpan<ImGuiTableColumn>    Columns;                    // Point within RawData[]
        ImSpan<ImGuiTableColumnIdx> DisplayOrderToIndex;        // Point within RawData[]. Store display order of columns (when not reordered, the values are 0...Count-1)
        ImSpan<ImGuiTableCellData>  RowCellData;                // Point within RawData[]. Store cells background requests for current row.
        ImBitArrayPtr               EnabledMaskByDisplayOrder;  // Column DisplayOrder -> IsEnabled map
        ImBitArrayPtr               EnabledMaskByIndex;         // Column Index -> IsEnabled map (== not hidden by user/api) in a format adequate for iterating column without touching cold data
        ImBitArrayPtr               VisibleMaskByIndex;         // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by user/api && not hidden by scrolling/cliprect)
        ImGuiTableFlags             SettingsLoadedFlags;        // Which data were loaded from the .ini file (e.g. when order is not altered we won't save order)
        int                         SettingsOffset;             // Offset in g.SettingsTables
        int                         LastFrameActive;
        int                         ColumnsCount;               // Number of columns declared in BeginTable()
        int                         CurrentRow;
        int                         CurrentColumn;
        ImS16                       InstanceCurrent;            // Count of BeginTable() calls with same ID in the same frame (generally 0). This is a little bit similar to BeginCount for a window, but multiple tables with the same ID are multiple tables, they are just synced.
        ImS16                       InstanceInteracted;         // Mark which instance (generally 0) of the same ID is being interacted with
        float                       RowPosY1;
        float                       RowPosY2;
        float                       RowMinHeight;               // Height submitted to TableNextRow()
        float                       RowCellPaddingY;            // Top and bottom padding. Reloaded during row change.
        float                       RowTextBaseline;
        float                       RowIndentOffsetX;
        ImGuiTableRowFlags          RowFlags : 16;              // Current row flags, see ImGuiTableRowFlags_
        ImGuiTableRowFlags          LastRowFlags : 16;
        int                         RowBgColorCounter;          // Counter for alternating background colors (can be fast-forwarded by e.g clipper), not same as CurrentRow because header rows typically don't increase this.
        ImU32                       RowBgColor[2];              // Background color override for current row.
        ImU32                       BorderColorStrong;
        ImU32                       BorderColorLight;
        float                       BorderX1;
        float                       BorderX2;
        float                       HostIndentX;
        float                       MinColumnWidth;
        float                       OuterPaddingX;
        float                       CellPaddingX;               // Padding from each borders. Locked in BeginTable()/Layout.
        float                       CellSpacingX1;              // Spacing between non-bordered cells. Locked in BeginTable()/Layout.
        float                       CellSpacingX2;
        float                       InnerWidth;                 // User value passed to BeginTable(), see comments at the top of BeginTable() for details.
        float                       ColumnsGivenWidth;          // Sum of current column width
        float                       ColumnsAutoFitWidth;        // Sum of ideal column width in order nothing to be clipped, used for auto-fitting and content width submission in outer window
        float                       ColumnsStretchSumWeights;   // Sum of weight of all enabled stretching columns
        float                       ResizedColumnNextWidth;
        float                       ResizeLockMinContentsX2;    // Lock minimum contents width while resizing down in order to not create feedback loops. But we allow growing the table.
        float                       RefScale;                   // Reference scale to be able to rescale columns on font/dpi changes.
        float                       AngledHeadersHeight;        // Set by TableAngledHeadersRow(), used in TableUpdateLayout()
        float                       AngledHeadersSlope;         // Set by TableAngledHeadersRow(), used in TableUpdateLayout()
        ImRect                      OuterRect;                  // Note: for non-scrolling table, OuterRect.Max.y is often FLT_MAX until EndTable(), unless a height has been specified in BeginTable().
        ImRect                      InnerRect;                  // InnerRect but without decoration. As with OuterRect, for non-scrolling tables, InnerRect.Max.y is "
        ImRect                      WorkRect;
        ImRect                      InnerClipRect;
        ImRect                      BgClipRect;                 // We use this to cpu-clip cell background color fill, evolve during the frame as we cross frozen rows boundaries
        ImRect                      Bg0ClipRectForDrawCmd;      // Actual ImDrawCmd clip rect for BG0/1 channel. This tends to be == OuterWindow->ClipRect at BeginTable() because output in BG0/BG1 is cpu-clipped
        ImRect                      Bg2ClipRectForDrawCmd;      // Actual ImDrawCmd clip rect for BG2 channel. This tends to be a correct, tight-fit, because output to BG2 are done by widgets relying on regular ClipRect.
        ImRect                      HostClipRect;               // This is used to check if we can eventually merge our columns draw calls into the current draw call of the current window.
        ImRect                      HostBackupInnerClipRect;    // Backup of InnerWindow->ClipRect during PushTableBackground()/PopTableBackground()
        ImGuiWindow* OuterWindow;                // Parent window for the table
        ImGuiWindow* InnerWindow;                // Window holding the table data (== OuterWindow or a child window)
        ImGuiTextBuffer             ColumnsNames;               // Contiguous buffer holding columns names
        ImDrawListSplitter* DrawSplitter;               // Shortcut to TempData->DrawSplitter while in table. Isolate draw commands per columns to avoid switching clip rect constantly
        ImGuiTableInstanceData      InstanceDataFirst;
        ImVector<ImGuiTableInstanceData>    InstanceDataExtra;  // FIXME-OPT: Using a small-vector pattern would be good.
        ImGuiTableColumnSortSpecs   SortSpecsSingle;
        ImVector<ImGuiTableColumnSortSpecs> SortSpecsMulti;     // FIXME-OPT: Using a small-vector pattern would be good.
        ImGuiTableSortSpecs         SortSpecs;                  // Public facing sorts specs, this is what we return in TableGetSortSpecs()
        ImGuiTableColumnIdx         SortSpecsCount;
        ImGuiTableColumnIdx         ColumnsEnabledCount;        // Number of enabled columns (<= ColumnsCount)
        ImGuiTableColumnIdx         ColumnsEnabledFixedCount;   // Number of enabled columns using fixed width (<= ColumnsCount)
        ImGuiTableColumnIdx         DeclColumnsCount;           // Count calls to TableSetupColumn()
        ImGuiTableColumnIdx         AngledHeadersCount;         // Count columns with angled headers
        ImGuiTableColumnIdx         HoveredColumnBody;          // Index of column whose visible region is being hovered. Important: == ColumnsCount when hovering empty region after the right-most column!
        ImGuiTableColumnIdx         HoveredColumnBorder;        // Index of column whose right-border is being hovered (for resizing).
        ImGuiTableColumnIdx         HighlightColumnHeader;      // Index of column which should be highlighted.
        ImGuiTableColumnIdx         AutoFitSingleColumn;        // Index of single column requesting auto-fit.
        ImGuiTableColumnIdx         ResizedColumn;              // Index of column being resized. Reset when InstanceCurrent==0.
        ImGuiTableColumnIdx         LastResizedColumn;          // Index of column being resized from previous frame.
        ImGuiTableColumnIdx         HeldHeaderColumn;           // Index of column header being held.
        ImGuiTableColumnIdx         LastHeldHeaderColumn;       // Index of column header being held from previous frame.
        ImGuiTableColumnIdx         ReorderColumn;              // Index of column being reordered. (not cleared)
        ImGuiTableColumnIdx         ReorderColumnDstOrder;      // Requested display order of column being reordered.
        ImGuiTableColumnIdx         LeftMostEnabledColumn;      // Index of left-most non-hidden column.
        ImGuiTableColumnIdx         RightMostEnabledColumn;     // Index of right-most non-hidden column.
        ImGuiTableColumnIdx         LeftMostStretchedColumn;    // Index of left-most stretched column.
        ImGuiTableColumnIdx         RightMostStretchedColumn;   // Index of right-most stretched column.
        ImGuiTableColumnIdx         ContextPopupColumn;         // Column right-clicked on, of -1 if opening context menu from a neutral/empty spot
        ImGuiTableColumnIdx         FreezeRowsRequest;          // Requested frozen rows count
        ImGuiTableColumnIdx         FreezeRowsCount;            // Actual frozen row count (== FreezeRowsRequest, or == 0 when no scrolling offset)
        ImGuiTableColumnIdx         FreezeColumnsRequest;       // Requested frozen columns count
        ImGuiTableColumnIdx         FreezeColumnsCount;         // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no scrolling offset)
        ImGuiTableColumnIdx         RowCellDataCurrent;         // Index of current RowCellData[] entry in current row
        ImGuiTableDrawChannelIdx    DummyDrawChannel;           // Redirect non-visible columns here.
        ImGuiTableDrawChannelIdx    Bg2DrawChannelCurrent;      // For Selectable() and other widgets drawing across columns after the freezing line. Index within DrawSplitter.Channels[]
        ImGuiTableDrawChannelIdx    Bg2DrawChannelUnfrozen;
        ImS8                        NavLayer;                   // ImGuiNavLayer at the time of BeginTable().
        bool                        IsLayoutLocked;             // Set by TableUpdateLayout() which is called when beginning the first row.
        bool                        IsInsideRow;                // Set when inside TableBeginRow()/TableEndRow().
        bool                        IsInitializing;
        bool                        IsSortSpecsDirty;
        bool                        IsUsingHeaders;             // Set when the first row had the ImGuiTableRowFlags_Headers flag.
        bool                        IsContextPopupOpen;         // Set when default context menu is open (also see: ContextPopupColumn, InstanceInteracted).
        bool                        DisableDefaultContextMenu;  // Disable default context menu. You may submit your own using TableBeginContextMenuPopup()/EndPopup()
        bool                        IsSettingsRequestLoad;
        bool                        IsSettingsDirty;            // Set when table settings have changed and needs to be reported into ImGuiTableSettings data.
        bool                        IsDefaultDisplayOrder;      // Set when display order is unchanged from default (DisplayOrder contains 0...Count-1)
        bool                        IsResetAllRequest;
        bool                        IsResetDisplayOrderRequest;
        bool                        IsUnfrozenRows;             // Set when we got past the frozen row.
        bool                        IsDefaultSizingPolicy;      // Set if user didn't explicitly set a sizing policy in BeginTable()
        bool                        IsActiveIdAliveBeforeTable;
        bool                        IsActiveIdInTable;
        bool                        HasScrollbarYCurr;          // Whether ANY instance of this table had a vertical scrollbar during the current frame.
        bool                        HasScrollbarYPrev;          // Whether ANY instance of this table had a vertical scrollbar during the previous.
        bool                        MemoryCompacted;
        bool                        HostSkipItems;              // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we will overwrite InnerWindow->SkipItem on a per-column basis

        ImGuiTable() { memset((void*)this, 0, sizeof(*this)); LastFrameActive = -1; }
        ~ImGuiTable() { IM_FREE(RawData); }
    };

    // sizeof() ~ 16
    struct ImGuiTableColumnSettings
    {
        float                   WidthOrWeight;
        ImGuiID                 UserID;
        ImGuiTableColumnIdx     Index;
        ImGuiTableColumnIdx     DisplayOrder;
        ImGuiTableColumnIdx     SortOrder;
        ImU8                    SortDirection : 2;
        ImS8                    IsEnabled : 2; // "Visible" in ini file
        ImU8                    IsStretch : 1;

        ImGuiTableColumnSettings()
        {
            WidthOrWeight = 0.0f;
            UserID = 0;
            Index = -1;
            DisplayOrder = SortOrder = -1;
            SortDirection = ImGuiSortDirection_None;
            IsEnabled = -1;
            IsStretch = 0;
        }
    };

    // This is designed to be stored in a single ImChunkStream (1 header followed by N ImGuiTableColumnSettings, etc.)
    struct ImGuiTableSettings
    {
        ImGuiID                     ID;                     // Set to 0 to invalidate/delete the setting
        ImGuiTableFlags             SaveFlags;              // Indicate data we want to save using the Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
        float                       RefScale;               // Reference scale to be able to rescale columns on font/dpi changes.
        ImGuiTableColumnIdx         ColumnsCount;
        ImGuiTableColumnIdx         ColumnsCountMax;        // Maximum number of columns this settings instance can store, we can recycle a settings instance with lower number of columns but not higher
        bool                        WantApply;              // Set when loaded from .ini data (to enable merging/loading .ini data into an already running context)

        ImGuiTableSettings() { memset((void*)this, 0, sizeof(*this)); }
        ImGuiTableColumnSettings* GetColumnSettings() { return (ImGuiTableColumnSettings*)(this + 1); }
    };

	// storage
    extern ImGuiTable*               CurrentTable;
    extern ImPool<ImGuiTable>        Tables;
    extern ImVector<ImGuiPtrOrIndex> CurrentTableStack;
    extern ImVector<float>           TablesLastTimeActive;       // Last used timestamp of each tables (SOA, for efficient GC)
    // extern ImVector<ImDrawChannel>   DrawChannelsTempMergeBuffer;
    extern ImChunkStream<ImGuiTableSettings>   SettingsTables;         // ImGuiTable .ini settings entries

	// functions
    bool BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags = 0, ImGuiWindowFlags subWindowFlags = 0, const ImVec2& outer_size = ImVec2(0,0), float inner_width = 0);
    void TableResetSettings(ImGuiTable* table);
    ImGuiTableSettings* TableSettingsFindByID(ImGuiID id);
    ImGuiTableSettings* TableGetBoundSettings(ImGuiTable* table);
    void TableSetColumnWidth(int column_n, float width);
    void TableSetupColumn(const char* label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_id);
    void TableNextRow(ImGuiTableRowFlags row_flags = 0, float row_min_height = 0);
    float TableGetColumnWidthAuto(ImGuiTable* table, ImGuiTableColumn* column);
    bool TableNextColumn();
    ImGuiTableSortSpecs* TableGetSortSpecs();
    void TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);
    void EndTable();
    const char* TableGetColumnName(const ImGuiTable* table, int column_n);
    void TableHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment = Alignment::Left);
    ImGuiSortDirection TableGetColumnNextSortDirection(ImGuiTableColumn* column);
    float GetColumnWidth(int column_index = -1);
    void AlignedTextColumn(Alignment alignment, const char* text, ...);
    void RegisterSettingsHandler(const char* name);
    void MenuItemTableColumnVisibility(ImGuiTable* table, int columnIdx);
    ImRect TableGetCellBgRect(const ImGuiTable* table, int column_n);
    int TableGetColumnIndex();
	float TableGetHeaderRowHeight();
	void TableSetupScrollFreeze(int columns, int rows);
    void TableFixDisplayOrder(ImGuiTable* table);
    void TableSetColumnDisplayOrder(ImGuiTable* table, int column_n, int dst_order);
    inline ImGuiTableInstanceData* TableGetInstanceData(ImGuiTable* table, int instance_no) { if (instance_no == 0) return &table->InstanceDataFirst; return &table->InstanceDataExtra[instance_no - 1]; }
    inline ImGuiID TableGetInstanceID(ImGuiTable* table, int instance_no) { return TableGetInstanceData(table, instance_no)->TableInstanceID; }
    float TableCalcMaxColumnWidth(const ImGuiTable* table, int column_n);
    void TableQueueSetColumnDisplayOrder(ImGuiTable* table, int column_n, int dst_order);
}

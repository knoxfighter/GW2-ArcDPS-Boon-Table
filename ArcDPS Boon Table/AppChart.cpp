#include "AppChart.h"

#include <algorithm>
#include <mutex>

// Includes/Defines to use, when enabling the custom aligned progressbar text.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui/imgui_internal.h"

void AppChart::Draw(const char* title, bool* p_open, Tracker& tracker, ImGuiWindowFlags flags = 0)
{
	ImGui::Begin(title, p_open, flags);

	// Settings on right-click-menu
	if (ImGui::BeginPopupContextWindow()) {
		// ImGui::MenuItem("Players", nullptr, &show_players);
		ImGui::MenuItem("Subgroups", nullptr, &show_subgroups);
		ImGui::MenuItem("Total", nullptr, &show_total);
		ImGui::MenuItem("Show value as progress bar", nullptr, &show_boon_as_progress_bar);
		ImGui::MenuItem("Paint by profession", nullptr, &show_colored);

		float cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cursorPosY + 4);
		ImGui::Text("Alignment");
		ImGui::SameLine();
		ImGui::SetCursorPosY(cursorPosY);
		if (ImGui::BeginCombo("###Alignment", alignment_text.c_str())) {
			alignmentSelectable(Alignment::Unaligned);
			alignmentSelectable(Alignment::Left);
			alignmentSelectable(Alignment::Center);
			alignmentSelectable(Alignment::Right);
			
			ImGui::EndCombo();
		}

		ImGui::EndPopup();
	}

	// columns: charname | subgroup | tracked_buffs
	const int columnCount = tracked_buffs.size() + 2;
	const int nameColumnId = columnCount - 2;
	const int subgroupColumnId = columnCount - 1;

	std::scoped_lock<std::mutex, std::mutex> lock(tracker.players_mtx, boons_mtx);
	
	if (ImGui::BeginTable("Table", columnCount, 
        ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | 
		ImGuiTableFlags_Sortable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody)) {
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder, 0, nameColumnId);
		ImGui::TableSetupColumn("Subgrp", 0, 0, subgroupColumnId);

		ImU32 i = 0;
		for (const BoonDef& trackedBuff : tracked_buffs) {
			int bufFlags = ImGuiTableColumnFlags_WidthFixed;
			if (!trackedBuff.is_relevant) {
				bufFlags |= ImGuiTableColumnFlags_DefaultHide;
			}
			if (i == 0) {
				bufFlags |= ImGuiTableColumnFlags_DefaultSort;
			}
			ImGui::TableSetupColumn(trackedBuff.name.c_str(), bufFlags, 80, i);

			++i;
		}

		ImGui::TableHeadersRow();

		// sorting
		if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs()) {
			// Sort our data if sort specs have been changed!
			if (sorts_specs->SpecsDirty)
				needSort = true;

			bool expected = true;
			if (needSort.compare_exchange_strong(expected, false)) {
				const bool descend = sorts_specs->Specs->SortDirection == ImGuiSortDirection_Descending;

				if (sorts_specs->Specs->ColumnUserID == nameColumnId) {
					// sort by account name.
					tracker.players.sort([descend](const Player& player1, const Player& player2) -> bool {
						std::string charName1 = player1.name;
						std::string charName2 = player2.name;
						std::transform(charName1.begin(), charName1.end(), charName1.begin(), [](unsigned char c) { return std::tolower(c); });
						std::transform(charName2.begin(), charName2.end(), charName2.begin(), [](unsigned char c) { return std::tolower(c); });

						if (descend) {
							bool res = charName1 < charName2;
							return res;
						} else {
							return charName1 > charName2;
						}
					});
				} else if (sorts_specs->Specs->ColumnUserID == subgroupColumnId) {
					// sort by subgroup
					tracker.players.sort([descend](const Player& player1, const Player& player2) {
						if (descend) {
							return player1.subgroup < player2.subgroup;
						} else {
							return player1.subgroup > player2.subgroup;
						}
					});
				}
				else {
					// sort by buff
					const ImGuiID buffId = sorts_specs->Specs->ColumnUserID;
					BoonDef buff = tracked_buffs[buffId];
					tracker.players.sort([descend, buff](const Player& player1, const Player& player2) {
						if (descend) {
							return player1.getBoonUptime(buff) < player2.getBoonUptime(buff);
						} else {
							return player1.getBoonUptime(buff) > player2.getBoonUptime(buff);
						}
					});
				}
				sorts_specs->SpecsDirty = false;
			}
		}
		
		// Show players
		for (Player player : tracker.players) {
			ImVec4 player_color = player.getProfessionColor();

			// charname
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(player.name.c_str());

			// subgroup
			ImGui::TableNextColumn();
			ImGui::Text("%d", player.subgroup);

			// buffs
			for (const BoonDef& trackedBuff : tracked_buffs) {
				ImGui::TableNextColumn();
				const float boonUptime = getPlayerDisplayValue(tracker, player, trackedBuff);

				buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth(), player_color);
			}
		}

		if (bShowSubgroups(tracker))
		{
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));

			for (uint8_t subgroup : tracker.subgroups) {
				ImGui::TableNextRow();

				// charname
				ImGui::TableNextColumn();
				ImGui::Text("Subgroup");

				// subgroup
				ImGui::TableNextColumn();
				ImGui::Text("%d", subgroup);

				// buffs
				for (const BoonDef& trackedBuff : tracked_buffs) {
					ImGui::TableNextColumn();
					const float boonUptime = tracker.getSubgroupBoonUptime(trackedBuff, subgroup);

					buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth());
				}
			}
		}

		if (bShowTotal())
		{
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));

			ImGui::TableNextRow();
			
			// charname
			ImGui::TableNextColumn();
			ImGui::Text("TOTAL");

			// subgroup
			ImGui::TableNextColumn();
			ImGui::Text("ALL");

			// buffs
			for (const BoonDef& trackedBuff : tracked_buffs) {
				ImGui::TableNextColumn();
				float averageBoonUptime = tracker.getAverageBoonUptime(trackedBuff);
				buffProgressBar(trackedBuff, averageBoonUptime, ImGui::GetColumnWidth());
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void AppChart::alignmentSelectable(Alignment select_alignment) {
	std::string new_alignment_text = to_string(select_alignment);
	if (ImGui::Selectable(new_alignment_text.c_str())) {
		alignment = select_alignment;
		alignment_text = new_alignment_text;
	}
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const {
	bool hidden_color = false;
	if (color.z == 0.f) hidden_color = true;
	if (show_boon_as_progress_bar)
	{
		if (show_colored && !hidden_color) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
		
		char label[10];
		if (current_buff.stacking_type == StackingType_intensity)
		{
			sprintf(label, "%.1f", current_boon_uptime);
			current_boon_uptime /= 25;
			// ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
			CustomProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
		}
		else
		{
			sprintf(label, "%.0f%%", 100*current_boon_uptime);
			// ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
			CustomProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
		}

		if (show_colored && !hidden_color) ImGui::PopStyleColor();
	}
	else
	{
		if (show_colored && !hidden_color) ImGui::PushStyleColor(ImGuiCol_Text, color);

		if (current_buff.stacking_type == StackingType_intensity)
		{
			//don't show the % for intensity stacking buffs
			ImGui::Text("%.1f", current_boon_uptime);
		}
		else
		{
			ImGui::Text("%.0f%%", 100*current_boon_uptime);
		}

		if (show_colored && !hidden_color) ImGui::PopStyleColor();
	}
}

// void AppChart::CustomProgressBar(float fraction, const ImVec2& size_arg, const char* overlay) const {
	// ImGui::ProgressBar(fraction, size_arg, overlay);
// }

// This code can be used to make the text over the progressBar centered.
// Moving from there, left/right alignment would be also easy to implement.
// This also uses imgui internals, which are likely to change between versions.
void AppChart::CustomProgressBar(float fraction, const ImVec2& size_arg, const char* overlay) const {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
	ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	// Render
	fraction = ImSaturate(fraction);
	ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
	bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
	const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
	ImGui::RenderRectFilledRangeH(window->DrawList, bb, ImGui::GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

	// Default displaying the fraction as percentage string, but user can override it
	char overlay_buf[32];
	if (!overlay)
	{
		ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
		overlay = overlay_buf;
	}

	ImVec2 overlay_size = ImGui::CalcTextSize(overlay, NULL);
	if (overlay_size.x > 0.0f) {
		switch (alignment) {
		case Alignment::Left:
			ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(0.f, 0.f), &bb);
			break;
		case Alignment::Center:
			ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(0.5f, 0.5f), &bb);
			break;
		case Alignment::Right: 
			ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(1.f, 0.f), &bb);
			break;
		default: 
			ImGui::RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f, 0.5f), &bb);
		}
	}
}

float AppChart::getPlayerDisplayValue(const Tracker& tracker, const Player& player, const BoonDef& boon)
{
	return player.getBoonUptime(boon);
}


void AppChart::setShowPlayers(bool new_show)
{
	show_players = new_show;
}

void AppChart::setShowSubgroups(bool new_show)
{
	show_subgroups = new_show;
}

void AppChart::setShowTotal(bool new_show)
{
	show_total = new_show;
}

void AppChart::setShowBoonAsProgressBar(bool new_show)
{
	show_boon_as_progress_bar = new_show;
}

void AppChart::setShowColored(bool new_colored) {
	show_colored = new_colored;
}

void AppChart::setAlignment(Alignment new_alignment) {
	alignment = new_alignment;
	alignment_text = to_string(new_alignment);
}

bool AppChart::bShowPlayers() const {
	return show_players;
}

bool AppChart::bShowSubgroups(const Tracker& tracker) const {
	return show_subgroups
		&& tracker.is_squad
		&& tracker.subgroups.size() > 1;
}

/**
 * DO NOT USE THIS. Use `AppChart::bShowSubgroups` instead.
 * This function is only to be used when writing the ini
 */
bool AppChart::getShowSubgroups() const {
	return show_subgroups;
}

bool AppChart::bShowTotal() const {
	return show_total;
}

bool AppChart::bShowBoonAsProgressBar() const {
	return show_boon_as_progress_bar;
}

bool AppChart::bShowColored() const {
	return show_colored;
}

Alignment AppChart::getAlignment() const {
	return alignment;
}

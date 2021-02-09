#include "AppChart.h"

#include <algorithm>
#include <mutex>

void AppChart::Draw(const char* title, bool* p_open, Tracker& tracker, ImGuiWindowFlags flags = 0)
{
	ImGui::Begin(title, p_open, flags);

	// header with settings
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Settings")) {
			// ImGui::MenuItem("Players", nullptr, &show_players);
			ImGui::MenuItem("Subgroups", nullptr, &show_subgroups);
			ImGui::MenuItem("Total", nullptr, &show_total);
			ImGui::MenuItem("Show value as progress bar", nullptr, &show_boon_as_progress_bar);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	// columns: charname | subgroup | tracked_buffs
	const int columnCount = tracked_buffs.size() + 2;
	const int nameColumnId = columnCount - 2;
	const int subgroupColumnId = columnCount - 1;

	std::scoped_lock<std::mutex, std::mutex> lock(tracker.players_mtx, boons_mtx);
	
	if (ImGui::BeginTable("Table", columnCount, 
                                                ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_RowBg)) {
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
							return player1.getBoonGeneration(buff) > player2.getBoonUptime(buff);
						}
					});
				}
				sorts_specs->SpecsDirty = false;
			}
		}
		
		// Show players
		for (Player player : tracker.players) {
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

				buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth());
			}
		}

		if (bShowSubgroups(tracker))
		{
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xffffffff);

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
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, 0xffffffff);
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

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width) const {
	if (show_boon_as_progress_bar)
	{
		char label[10];
		if (current_buff.stacking_type == StackingType_intensity)
		{
			sprintf(label, "%.1f", current_boon_uptime);
			current_boon_uptime /= 25;
			ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
		}
		else
		{
			sprintf(label, "%.0f%%", 100*current_boon_uptime);
			ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
		}
	}
	else
	{
		if (current_buff.stacking_type == StackingType_intensity)
		{
			//don't show the % for intensity stacking buffs
			ImGui::Text("%.1f", current_boon_uptime);
		}
		else
		{
			ImGui::Text("%.0f%%", 100*current_boon_uptime);
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

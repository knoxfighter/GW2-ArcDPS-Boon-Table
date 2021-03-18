#include "AppChart.h"

#include <algorithm>
#include <mutex>

#include "Lang.h"
#include "extension/Widgets.h"
#include "imgui/imgui_internal.h"

void AppChart::Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags = 0) {
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.back());

	flags |= ImGuiWindowFlags_NoCollapse;
	if (bSizeToContent()) {
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
	}
	std::string windowName = lang.translate(LangKey::WindowHeader);
	windowName.append("##Boon Table");
	ImGui::Begin(windowName.c_str(), p_open, flags);

	// Settings on right-click-menu
	if (ImGui::BeginPopupContextWindow()) {
		ImGui::MenuItem(lang.translate(LangKey::SettingsPlayers).c_str(), nullptr, &show_players);
		ImGui::MenuItem(lang.translate(LangKey::SettingsSubgroups).c_str(), nullptr, &show_subgroups);
		ImGui::MenuItem(lang.translate(LangKey::SettingsTotal).c_str(), nullptr, &show_total);
		ImGui::MenuItem(lang.translate(LangKey::SettingsNPC).c_str(), nullptr, &show_npcs);
		ImGui::MenuItem(lang.translate(LangKey::SettingsShowProgressBar).c_str(), nullptr, &show_boon_as_progress_bar);
		ImGui::MenuItem(lang.translate(LangKey::SettingsAlwaysResize).c_str(), nullptr, &size_to_content);
		ImGui::MenuItem(lang.translate(LangKey::SettingsAlternatingRow).c_str(), nullptr, &alternating_row_bg);
		ImGui::MenuItem(lang.translate(LangKey::SettingsShowLabel).c_str(), nullptr, &show_label);

		float cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cursorPosY + 4);
		ImGui::Text(lang.translate(LangKey::SettingsColoringMode).c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosY(cursorPosY);
		if (ImGui::BeginCombo("###ShowColored", to_string(show_colored).c_str())) {
			ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::Uncolored);
			ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::ByProfession);
			ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::ByPercentage);

			ImGui::EndCombo();
		}

		cursorPosY = ImGui::GetCursorPosY();
		ImGui::SetCursorPosY(cursorPosY + 4);
		ImGui::Text(lang.translate(LangKey::SettingsAlignment).c_str());
		ImGui::SameLine();
		ImGui::SetCursorPosY(cursorPosY);
		if (ImGui::BeginCombo("###Alignment", to_string(alignment).c_str())) {
			ImGuiEx::Selectable(alignment, Alignment::Unaligned);
			ImGuiEx::Selectable(alignment, Alignment::Left);
			ImGuiEx::Selectable(alignment, Alignment::Center);
			ImGuiEx::Selectable(alignment, Alignment::Right);

			ImGui::EndCombo();
		}

		ImGui::EndPopup();
	}

	// columns: charname | subgroup | tracked_buffs
	const int columnCount = tracked_buffs.size() + 2;
	const int nameColumnId = columnCount - 2;
	const int subgroupColumnId = columnCount - 1;

	std::scoped_lock<std::mutex, std::mutex, std::mutex> lock(tracker.players_mtx, tracker.npcs_mtx, boons_mtx);

	int tableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_ContextMenuInBody |
		ImGuiTableFlags_BordersInnerH |
		ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;

	if (bAlternatingRowBg()) {
		tableFlags |= ImGuiTableFlags_RowBg;
	}

	if (ImGui::BeginTable("Table", columnCount, tableFlags)) {
		/*
		 * HEADER
		 */
		std::string charName = lang.translate(LangKey::NameColumnHeader);
		std::string subgroupName = lang.translate(LangKey::SubgroupColumnHeader);


		ImGui::TableSetupColumn(charName.c_str(), 0, 0, nameColumnId);
		ImGui::TableSetupColumn(subgroupName.c_str(), 0, 0, subgroupColumnId);

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

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

		// accountname header
		if (ImGui::TableNextColumn())
			ImGuiEx::TableHeader(charName.c_str(), true, nullptr);

		// subgroup header
		if (ImGui::TableNextColumn())
			ImGuiEx::TableHeader(subgroupName.c_str(), true, nullptr);

		for (const BoonDef& trackedBuff : tracked_buffs) {
			if (ImGui::TableNextColumn()) {
				ImGuiEx::TableHeader(trackedBuff.name.c_str(), show_label, trackedBuff.icon->texture, alignment);
			}
		}


		/*
		 * SORTING
		 */
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
				} else {
					// sort by buff
					const ImGuiID buffId = sorts_specs->Specs->ColumnUserID;
					const BoonDef& buff = tracked_buffs[buffId];
					tracker.players.sort([descend, &buff](const Player& player1, const Player& player2) {
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

		/*
		 * PLAYERS
		 */
		if (bShowPlayers()) {
			for (Player player : tracker.players) {
				ImVec4 player_color = player.getColor();

				// charname
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				std::string name_string = player.name; // +(player.in_combat ? "*" : "") + +" (" + std::to_string(player.id) + ")";
				ImGui::Text(name_string.c_str());

				// subgroup
				if (ImGui::TableNextColumn()) {
					ImGuiEx::AlignedTextColumn(alignment, "%d", player.subgroup);
				}

				// buffs
				for (const BoonDef& trackedBuff : tracked_buffs) {
					if (ImGui::TableNextColumn()) {
						const float boonUptime = getEntityDisplayValue(tracker, player, trackedBuff);

						buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth(), player);
					}
				}
			}
		}

		/*
		 * SUBGROUPS
		 */
		if (bShowSubgroups(tracker)) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));

			for (uint8_t subgroup : tracker.subgroups) {
				ImGui::TableNextRow();

				// charname
				ImGui::TableNextColumn();
				ImGui::Text(lang.translate(LangKey::SubgroupNameColumnValue).c_str());

				// subgroup
				if (ImGui::TableNextColumn()) {
					ImGuiEx::AlignedTextColumn(alignment, "%d", subgroup);
				}

				// buffs
				for (const BoonDef& trackedBuff : tracked_buffs) {
					if (ImGui::TableNextColumn()) {
						float boonUptime = tracker.getSubgroupBoonUptime(trackedBuff, subgroup);

						buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth());
					}
				}
			}
		}

		/*
		 * TOTALS
		 */
		if (bShowTotal()) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));

			ImGui::TableNextRow();

			// charname
			ImGui::TableNextColumn();
			ImGui::Text(lang.translate(LangKey::TotalNameColumnValue).c_str());

			// subgroup
			if (ImGui::TableNextColumn()) {
				ImGuiEx::AlignedTextColumn(alignment, lang.translate(LangKey::TotalSubgroupColumnValue).c_str());
			}

			// buffs
			for (const BoonDef& trackedBuff : tracked_buffs) {
				if (ImGui::TableNextColumn()) {
					float averageBoonUptime = tracker.getAverageBoonUptime(trackedBuff);
					buffProgressBar(trackedBuff, averageBoonUptime, ImGui::GetColumnWidth());
				}
			}
		}

		/*
		 * NPCs
		 */
		if (bShowNPCs() && !tracker.npcs.empty()) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));
			for (NPC npc : tracker.npcs) {
				ImVec4 npc_color = npc.getColor();

				// charname
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				std::string name_string = npc.name; // +(npc.in_combat ? "*" : "") + " (" + std::to_string(npc.id) + ")";
				ImGui::Text(name_string.c_str());

				// subgroup
				if (ImGui::TableNextColumn()) {
					ImGuiEx::AlignedTextColumn(alignment, lang.translate(LangKey::NPCSubgroupColumnValue).c_str());
				}

				// buffs
				for (const BoonDef& trackedBuff : tracked_buffs) {
					if (ImGui::TableNextColumn()) {
						const float boonUptime = getEntityDisplayValue(tracker, npc, trackedBuff);

						buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth(), npc);
					}
				}
			}
		}


		ImGui::EndTable();
	}

	ImGui::End();
	ImGui::PopFont();
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const {
	bool hidden_color = false;
	if (color.w == 0.f) hidden_color = true;
	if (show_boon_as_progress_bar) {
		if (show_colored != ProgressBarColoringMode::Uncolored && !hidden_color) ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);

		char label[10];
		if (current_buff.stacking_type == StackingType_intensity) {
			sprintf(label, "%.1f", current_boon_uptime);
			current_boon_uptime /= 25;
			// ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
			ImGuiEx::AlignedProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label, alignment);
		} else {
			sprintf(label, "%.0f%%", 100 * current_boon_uptime);
			// ImGui::ProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label);
			ImGuiEx::AlignedProgressBar(current_boon_uptime, ImVec2(width, ImGui::GetFontSize()), label, alignment);
		}

		if (show_colored != ProgressBarColoringMode::Uncolored && !hidden_color) ImGui::PopStyleColor();
	} else {
		if (show_colored != ProgressBarColoringMode::Uncolored && !hidden_color) ImGui::PushStyleColor(ImGuiCol_Text, color);

		if (current_buff.stacking_type == StackingType_intensity) {
			//don't show the % for intensity stacking buffs
			ImGuiEx::AlignedTextColumn(alignment, "%.1f", current_boon_uptime);
		} else {
			ImGuiEx::AlignedTextColumn(alignment, "%.0f%%", 100 * current_boon_uptime);
		}

		if (show_colored != ProgressBarColoringMode::Uncolored && !hidden_color) ImGui::PopStyleColor();
	}
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width) {
	switch (show_colored) {
	case ProgressBarColoringMode::ByPercentage:
		{
			float percentage = 0;
			if (current_buff.stacking_type == StackingType_intensity) {
				percentage = current_boon_uptime / 25;
			} else {
				percentage = current_boon_uptime;
			}
			// ImVec4 color((1 - percentage) * 255, 125/*percentage * 255*/, 0, 1);
			ImVec4 color(1 - percentage, percentage, 0, 0.75);
			buffProgressBar(current_buff, current_boon_uptime, width, color);
			break;
		}
	default: buffProgressBar(current_buff, current_boon_uptime, width, ImVec4(0, 0, 0, 0));
	}
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Entity& entity) const {
	switch (show_colored) {
	case ProgressBarColoringMode::ByProfession:
		buffProgressBar(current_buff, current_boon_uptime, width, entity.getColor());
		break;
	case ProgressBarColoringMode::ByPercentage:
		{
			float percentage = 0;
			if (current_buff.stacking_type == StackingType_intensity) {
				percentage = current_boon_uptime / 25;
			} else {
				percentage = current_boon_uptime;
			}
			ImVec4 color(1 - percentage, percentage, 0, 0.75);
			buffProgressBar(current_buff, current_boon_uptime, width, color);
			break;
		}
	default:
		buffProgressBar(current_buff, current_boon_uptime, width, ImVec4(0, 0, 0, 0));
	}
}

float AppChart::getEntityDisplayValue(const Tracker& tracker, const Entity& entity, const BoonDef& boon) {
	return entity.getBoonUptime(boon);
}


void AppChart::setShowPlayers(bool new_show) {
	show_players = new_show;
}

void AppChart::setShowSubgroups(bool new_show) {
	show_subgroups = new_show;
}

void AppChart::setShowTotal(bool new_show) {
	show_total = new_show;
}

void AppChart::setShowNPCs(bool new_show) {
	show_npcs = new_show;
}

void AppChart::setShowBoonAsProgressBar(bool new_show) {
	show_boon_as_progress_bar = new_show;
}

void AppChart::setShowColored(ProgressBarColoringMode new_colored) {
	show_colored = new_colored;
}

void AppChart::setSizeToContent(bool new_size_to_content) {
	size_to_content = new_size_to_content;
}

void AppChart::setAlternatingRowBg(bool new_alternating_row_bg) {
	alternating_row_bg = new_alternating_row_bg;
}

void AppChart::setAlignment(Alignment new_alignment) {
	alignment = new_alignment;
}

void AppChart::setShowLabel(bool new_show) {
	show_label = new_show;
}

bool AppChart::bShowPlayers() const {
	return show_players;
}

bool AppChart::bShowNPCs() const {
	return show_npcs;
}

bool AppChart::bShowLabel() const {
	return show_label;
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

ProgressBarColoringMode AppChart::getShowColored() const {
	return show_colored;
}

bool AppChart::bSizeToContent() const {
	return size_to_content;
}

bool AppChart::bAlternatingRowBg() const {
	return alternating_row_bg;
}

Alignment AppChart::getAlignment() const {
	return alignment;
}

#include "AppChart.h"

#include <algorithm>
#include <mutex>

#include "Lang.h"
#include "Settings.h"
#include "extension/Widgets.h"
#include "imgui/imgui_internal.h"

void AppChart::Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags = 0) {
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.back());

	flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	SizingPolicy sizingPolicy = settings.getSizingPolicy();
	switch (sizingPolicy) {
	case SizingPolicy::SizeToContent:
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
		break;
	}
	if (settings.isHideHeader()) {
		flags |= ImGuiWindowFlags_NoTitleBar;
	}
	std::string windowName = lang.translate(LangKey::WindowHeader);
	windowName.append("##Boon Table");
	ImGui::Begin(windowName.c_str(), p_open, flags);


	// columns: charname | subgroup | tracked_buffs
	const int columnCount = tracked_buffs.size() + 2;
	const int nameColumnId = columnCount - 2;
	const int subgroupColumnId = columnCount - 1;

	std::scoped_lock<std::mutex, std::mutex, std::mutex> lock(tracker.players_mtx, tracker.npcs_mtx, boons_mtx);

	int tableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable |
		ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_BordersInnerH;

	switch (sizingPolicy) {
	case SizingPolicy::ManualWindowSize:
	case SizingPolicy::SizeToContent:
		tableFlags |= ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
		break;
	case SizingPolicy::SizeContentToWindow:
		tableFlags |= ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY;
		break;
	}

	if (settings.isAlternatingRowBg()) {
		tableFlags |= ImGuiTableFlags_RowBg;
	}

	if (ImGui::BeginTable("Table", columnCount, tableFlags)) {
		Alignment alignment = settings.getAlignment();
		
		/*
		 * HEADER
		 */
		std::string charName = lang.translate(LangKey::NameColumnHeader);
		std::string subgroupName = lang.translate(LangKey::SubgroupColumnHeader);

		ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn(charName.c_str(), columnFlags, 0, nameColumnId);
		ImGui::TableSetupColumn(subgroupName.c_str(), columnFlags, 0, subgroupColumnId);

		ImU32 i = 0;
		for (const BoonDef& trackedBuff : tracked_buffs) {
			int bufFlags = ImGuiTableColumnFlags_NoResize;
			if (!trackedBuff.is_relevant) {
				bufFlags |= ImGuiTableColumnFlags_DefaultHide;
			}
			if (i == 0) {
				bufFlags |= ImGuiTableColumnFlags_DefaultSort;
			}
			float init_width = 80.f;
			if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
				init_width = settings.getBoonColumnWidth();
			}
			ImGui::TableSetupColumn(trackedBuff.name.c_str(), bufFlags, init_width, i);

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
				ImGuiEx::TableHeader(trackedBuff.name.c_str(), settings.isShowLabel(), trackedBuff.icon->texture, alignment);
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
		if (settings.isShowPlayers()) {
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
		if (settings.isShowSubgroups(tracker)) {
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
		if (settings.isShowTotal()) {
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
		if (settings.isShowNpcs() && !tracker.npcs.empty()) {
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
	ProgressBarColoringMode show_colored = settings.getShowColored();
	Alignment alignment = settings.getAlignment();

	bool hidden_color = false;
	if (color.w == 0.f) hidden_color = true;

	if (settings.isShowBoonAsProgressBar()) {
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
	switch (settings.getShowColored()) {
	case ProgressBarColoringMode::ByPercentage:
		{
			float percentage = 0;
			if (current_buff.stacking_type == StackingType_intensity) {
				percentage = current_boon_uptime / 25;
			} else {
				percentage = current_boon_uptime;
			}
			// ImVec4 color((1 - percentage) * 255, 125/*percentage * 255*/, 0, 1);
			ImVec4 color(1 - percentage, percentage, 0, 110.f / 255.f);
			buffProgressBar(current_buff, current_boon_uptime, width, color);
			break;
		}
	default: buffProgressBar(current_buff, current_boon_uptime, width, ImVec4(0, 0, 0, 0));
	}
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Entity& entity) const {
	switch (settings.getShowColored()) {
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

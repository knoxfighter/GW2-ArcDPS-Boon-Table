#include "AppChart.h"

#include <algorithm>
#include <mutex>
#include <ranges>

#include "Lang.h"
#include "Settings.h"
#include "SettingsUI.h"
#include "extension/Widgets.h"

AppChartsContainer charts;

void AppChart::Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags = 0) {
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts.back());

	flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	SizingPolicy sizingPolicy = settings.getSizingPolicy(index);
	switch (sizingPolicy) {
	case SizingPolicy::SizeToContent:
		flags |= ImGuiWindowFlags_AlwaysAutoResize;
		break;
	}
	if (settings.isHideHeader(index)) {
		flags |= ImGuiWindowFlags_NoTitleBar;
	}

	std::string windowName = lang.translate(LangKey::WindowHeader);
	windowName.append("##Boon Table");
	if (index > 0)
		windowName.append(std::to_string(index));
	ImGui::Begin(windowName.c_str(), p_open, flags);

	/**
	 * Settings UI
	 */
	if (ImGuiEx::BeginPopupContextWindow(nullptr, 1, ImGuiHoveredFlags_ChildWindows)) {
		settingsUi.Draw(imGuiTable, index);

		ImGui::EndPopup();
	}

	// columns: charname | subgroup | tracked_buffs
	const int columnCount = tracked_buffs.size() + 3;
	const unsigned int nameColumnId = columnCount - 3;
	const unsigned int subgroupColumnId = columnCount - 2;
	const unsigned int above90ColumnId = columnCount - 1;

	// we have to get it here, cause it will lock tracker.players_mtx itself (which causes a crash, when it is already locked)
	Player* self_player = tracker.getPlayer(2000);

	std::scoped_lock<std::mutex, std::mutex, std::mutex> lock(tracker.players_mtx, tracker.npcs_mtx, boons_mtx);

	int tableFlags = ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_NoBordersInBody;

	switch (sizingPolicy) {
	case SizingPolicy::ManualWindowSize:
	case SizingPolicy::SizeToContent:
		tableFlags |= ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY;
		break;
	case SizingPolicy::SizeContentToWindow:
		tableFlags |= ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY;
		break;
	}

	if (settings.isAlternatingRowBg(index)) {
		tableFlags |= ImGuiTableFlags_RowBg;
	}

	if (ImGui::BeginTable("Table", columnCount, tableFlags)) {
		ImGuiContext& imGuiContext = *GImGui;
		imGuiTable = imGuiContext.CurrentTable;
		
		Alignment alignment = settings.getAlignment(index);
		bool showLabel = settings.isShowLabel(index);

		/*
		 * HEADER
		 */
		std::string charName = lang.translate(LangKey::NameColumnHeader);
		std::string subgroupName = lang.translate(LangKey::SubgroupColumnHeader);

		ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn(charName.c_str(), columnFlags, 0, nameColumnId);
		ImGui::TableSetupColumn(subgroupName.c_str(), columnFlags, 0, subgroupColumnId);

		float init_width = 80.f;
		if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
			init_width = settings.getBoonColumnWidth(index);
		}
		
		ImU32 i = 0;
		for (const BoonDef& trackedBuff : tracked_buffs) {
			int bufFlags = ImGuiTableColumnFlags_NoResize;
			if (!trackedBuff.is_relevant) {
				bufFlags |= ImGuiTableColumnFlags_DefaultHide;
			}
			if (i == 0) {
				bufFlags |= ImGuiTableColumnFlags_DefaultSort;
			}
			
			ImGui::TableSetupColumn(trackedBuff.name.c_str(), bufFlags, init_width, i);

			++i;
		}

		ImGui::TableSetupColumn(above90BoonDef->name.c_str(), ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_DefaultHide, init_width, above90ColumnId);

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

		// accountname header
		if (ImGui::TableNextColumn())
			ImGuiEx::TableHeader(charName.c_str(), true, nullptr);

		// subgroup header
		if (ImGui::TableNextColumn())
			ImGuiEx::TableHeader(subgroupName.c_str(), true, nullptr);

		// buff headers
		for (const BoonDef& trackedBuff : tracked_buffs) {
			if (ImGui::TableNextColumn()) {
				ImGuiEx::TableHeader(trackedBuff.name.c_str(), showLabel, trackedBuff.icon->texture, alignment);
			}
		}

		// above90 header
		if (ImGui::TableNextColumn()) {
			ImGuiEx::TableHeader(above90BoonDef->name.c_str(), showLabel, above90BoonDef->icon->texture, alignment);
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
					std::sort(playerOrder.begin(), playerOrder.end(), [descend, &tracker](const size_t& playerIdx1, const size_t& playerIdx2) {
						std::string charName1 = tracker.players.at(playerIdx1).name;
						std::string charName2 = tracker.players.at(playerIdx2).name;
						std::transform(charName1.begin(), charName1.end(), charName1.begin(), [](unsigned char c) { return std::tolower(c); });
						std::transform(charName2.begin(), charName2.end(), charName2.begin(), [](unsigned char c) { return std::tolower(c); });

						if (descend) {
							bool res = charName1 < charName2;
							return res;
						}
						else {
							return charName1 > charName2;
						}
					});
				} else if (sorts_specs->Specs->ColumnUserID == subgroupColumnId) {
					// sort by subgroup
					std::sort(playerOrder.begin(), playerOrder.end(), [descend, &tracker](const size_t& playerIdx1, const size_t& playerIdx2) {
						const Player& player1 = tracker.players.at(playerIdx1);
						const Player& player2 = tracker.players.at(playerIdx2);
						if (descend) {
							return player1.subgroup < player2.subgroup;
						} else {
							return player1.subgroup > player2.subgroup;
						}
					});
				} else if (sorts_specs->Specs->ColumnUserID == above90ColumnId) {
					// sort by above 90% hp
					std::sort(playerOrder.begin(), playerOrder.end(), [descend, &tracker](const size_t& playerIdx1, const size_t& playerIdx2) {
						const Player& player1 = tracker.players.at(playerIdx1);
						const Player& player2 = tracker.players.at(playerIdx2);
						if (descend) {
							return player1.getOver90() < player2.getOver90();
						}
						else {
							return player1.getOver90() > player2.getOver90();
						}
					});
				} else {
					// sort by buff
					const ImGuiID buffId = sorts_specs->Specs->ColumnUserID;
					const BoonDef& buff = tracked_buffs[buffId];
					std::sort(playerOrder.begin(), playerOrder.end(), [descend, &tracker, &buff](const size_t& playerIdx1, const size_t& playerIdx2) {
						const Player& player1 = tracker.players.at(playerIdx1);
						const Player& player2 = tracker.players.at(playerIdx2);
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
		if (settings.isShowPlayers(index)) {
			bool onlySubgroup = settings.isShowOnlySubgroup(index);
			auto group_filter = [&self_player, onlySubgroup](const Player& player) {
				if (self_player && onlySubgroup) {
					uint8_t subgroup = self_player->subgroup;
					return player.subgroup == subgroup;
				}
				return true;
			};
			for (const Player& player : tracker.players | std::views::filter(group_filter)) {
				// charname
				if (player.self) {
					ImGui::PushStyleColor(ImGuiCol_Text, settings.getSelfColor());
				}
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				std::string name_string = player.name; // +(player.in_combat ? "*" : "") + +" (" + std::to_string(player.id) + ")";
				ImGui::TextUnformatted(name_string.c_str());

				// subgroup
				if (ImGui::TableNextColumn()) {
					ImGuiEx::AlignedTextColumn(alignment, "%d", player.subgroup);
				}

				if (player.self) {
					ImGui::PopStyleColor();
				}

				// buffs
				for (const BoonDef& trackedBuff : tracked_buffs) {
					if (ImGui::TableNextColumn()) {
						const float boonUptime = getEntityDisplayValue(tracker, player, trackedBuff);

						buffProgressBar(trackedBuff, boonUptime, ImGui::GetColumnWidth(), player);
					}
				}

				// over90
				if (ImGui::TableNextColumn()) {
					const float above90 = player.getOver90();
					buffProgressBar(*above90BoonDef, above90, ImGui::GetColumnWidth(), player);
				}
			}
		}

		/*
		 * SUBGROUPS
		 */
		if (settings.isShowSubgroups(tracker, index)) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));

			bool onlySubgroup = settings.isShowOnlySubgroup(index);
			auto group_filter = [&self_player, onlySubgroup](const uint8_t& subgroup) {
				if (self_player && onlySubgroup) {
					return subgroup == self_player->subgroup;
				}
				return true;
			};
			
			for (uint8_t subgroup : tracker.subgroups | std::views::filter(group_filter)) {
				ImGui::TableNextRow();

				// charname
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(lang.translate(LangKey::SubgroupNameColumnValue).c_str());

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

				// above 90
				if (ImGui::TableNextColumn()) {
					const float above90 = tracker.getSubgroupOver90(subgroup);
					buffProgressBar(*above90BoonDef, above90, ImGui::GetColumnWidth());
				}
			}
		}

		/*
		 * TOTALS
		 */
		if (settings.isShowTotal(index)) {
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

			// above90
			if (ImGui::TableNextColumn()) {
				const float above90 = tracker.getAverageOver90();
				buffProgressBar(*above90BoonDef, above90, ImGui::GetColumnWidth());
			}
		}

		/*
		 * NPCs
		 */
		if (settings.isShowNpcs(index) && !tracker.npcs.empty()) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Separator));
			for (const NPC& npc : tracker.npcs) {
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
		
				// above 90
				if (ImGui::TableNextColumn()) {
					const float above90 = npc.getOver90();
					buffProgressBar(*above90BoonDef, above90, ImGui::GetColumnWidth());
				}
			}
		}

		ImGui::EndTable();
	}

	ImGui::End();

	ImGui::PopFont();
}

void AppChart::buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const {
	ProgressBarColoringMode show_colored = settings.getShowColored(index);
	Alignment alignment = settings.getAlignment(index);

	bool hidden_color = false;
	if (color.w == 0.f) hidden_color = true;

	if (settings.isShowBoonAsProgressBar(index)) {
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
		if (show_colored != ProgressBarColoringMode::Uncolored && !hidden_color) {
			color.w = 1;
			ImGui::PushStyleColor(ImGuiCol_Text, color);
		}

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
	switch (settings.getShowColored(index)) {
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
	switch (settings.getShowColored(index)) {
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
			ImVec4 color(1 - percentage, percentage, 0, (float)125/255);
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

void AppChart::removePlayer(size_t playerIndex) {
	playerOrder.erase(std::remove(playerOrder.begin(), playerOrder.end(), playerIndex), playerOrder.end());
}

void AppChart::addPlayer(size_t playerIndex) {
	playerOrder.emplace_back(playerIndex);
}

void AppChartsContainer::removePlayer(size_t playerIndex) {
	for (AppChart& chart : charts) {
		chart.removePlayer(playerIndex);
	}
}

void AppChartsContainer::addPlayer(size_t playerIndex) {
	for (AppChart& chart : charts) {
		chart.addPlayer(playerIndex);
	}
}

void AppChartsContainer::sortNeeded() {
	for (AppChart& chart : charts) {
		chart.needSort = true;
	}
}

void AppChartsContainer::drawAll(Tracker& tracker, ImGuiWindowFlags flags) {
	for (AppChart& chart : charts) {
		bool& showChart = settings.isShowChart(chart.index);
		if (showChart) {
			chart.Draw(&showChart, tracker, flags);
		}
	}
}

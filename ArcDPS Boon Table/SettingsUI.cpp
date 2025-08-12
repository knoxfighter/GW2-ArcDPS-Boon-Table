#include "SettingsUI.h"

#include "AppChart.h"
#include "Helpers.h"
#include "History.h"
#include "Lang.h"
#include "Settings.h"
#include "SettingsUIGlobal.h"
#include "extension/Widgets.h"

SettingsUI settingsUi;

namespace Table = ImGuiEx::BigTable;

void SettingsUI::Draw(Table::ImGuiTable* table, int tableIndex, ImGuiWindow* currentRootWindow) {
	if (ImGui::IsWindowAppearing()) {
		initialize(tableIndex);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsHistory).c_str())) {
		ImVec4* arc_colors[5];
		arc_export_e5(arc_colors);
		
		if (settings.tables[tableIndex].current_history != 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, arc_colors[0][CCOL_LGREY]);
		}
		ImGui::TextUnformatted(lang.translate(LangKey::SettingsHistoryCurrent).c_str());
		if (settings.tables[tableIndex].current_history != 0) {
			ImGui::PopStyleColor();
		}
		if (ImGui::IsItemClicked()) {
			settings.tables[tableIndex].current_history = 0;
		}

		// 1-based, 0 = current
		int historyIndex = 1;
		for (const TrackerHistory& trackerHistory : history) {
			auto duration = trackerHistory.getDuration();

			// "<starttime>, <mm>m<ss.uu>s (<name>)"
			// starttime: <hh>:<mm>:<ss>
			std::stringstream ss;
			auto starttime = trackerHistory.getStarttime();
			ss << (starttime.hours() % 24).count() << ":";
			ss << starttime.minutes().count() << ":";
			ss << starttime.seconds().count() << ",";
			if (duration>= 60) {
				ss << " " << duration/60 << "m";
			}
			ss << " " << duration%60 << "s";
			ss << " (" << trackerHistory.getLogName() << ")";

			if (settings.tables[tableIndex].current_history != historyIndex) {
				ImGui::PushStyleColor(ImGuiCol_Text, arc_colors[0][CCOL_LGREY]);
			}
			ImGui::TextUnformatted(ss.str().c_str());
			if (settings.tables[tableIndex].current_history != historyIndex) {
				ImGui::PopStyleColor();
			}
			if (ImGui::IsItemClicked()) {
				settings.tables[tableIndex].current_history = historyIndex;
			}

			++historyIndex;
		}
		
		ImGui::EndMenu();
	}

	ImGui::Checkbox(lang.translate(LangKey::SettingsSelfOnTop).c_str(), &settings.tables[tableIndex].show_self_on_top);
	ImGui::Checkbox(lang.translate(LangKey::SettingsPlayers).c_str(), &settings.tables[tableIndex].show_players);
	ImGui::Checkbox(lang.translate(LangKey::SettingsSubgroups).c_str(), &settings.tables[tableIndex].show_subgroups);
	ImGui::Checkbox(lang.translate(LangKey::SettingsTotal).c_str(), &settings.tables[tableIndex].show_total);
	// ImGui::Checkbox(lang.translate(LangKey::SettingsNPC).c_str(), &settings.tables[tableIndex].show_npcs);

	ImGui::Separator();
	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsColumnSetup).c_str(), table != nullptr)) {
		ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

		// username
		Table::MenuItemTableColumnVisibility(table, 0);
		// subgroup
		Table::MenuItemTableColumnVisibility(table, 1);

		// Submenus for controlling visibility
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeBoon).c_str(), BoonType_boon, 2))
			ImGui::EndMenu();
		// if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeWarrior).c_str(), BoonType_Warrior, 2))
		// 	ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeRevenant).c_str(), BoonType_Revenant, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeGuardian).c_str(), BoonType_Guardian, 2))
			ImGui::EndMenu();
		// if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeEngineer).c_str(), BoonType_Engineer, 2))
		// 	ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeRanger).c_str(), BoonType_Ranger, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeElementalist).c_str(), BoonType_Elementalist, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeMesmer).c_str(), BoonType_Mesmer, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeNecromancer).c_str(), BoonType_Necromancer, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeAura).c_str(), BoonType_Aura, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeRelic).c_str(), BoonType_Relic, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeOther).c_str(), BoonType_other, 2)) {
			Table::MenuItemTableColumnVisibility(table, tracked_buffs.size() + 2);
			ImGui::EndMenu();
		}

		if (ImGui::Button(lang.translate(LangKey::SettingsResetTableColumns).c_str())) {
			Table::TableResetSettings(table);
		}

		ImGui::PopItemFlag();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsDisplay).c_str())) {
		if (directxVersion == 9) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			bool tmp = true;
			ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &tmp);
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();
		} else {
			ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.tables[tableIndex].show_label);
		}

		std::string maxDisplayedInputLabel = lang.translate(LangKey::SettingsMaxDisplayed);
		maxDisplayedInputLabel.append("##maxDisplayedInput");
		ImGui::InputInt(maxDisplayedInputLabel.c_str(), &settings.tables[tableIndex].max_displayed);
		ImGui::SameLine();
		HelpMarker(lang.translate(LangKey::SettingsMaxDisplayedPopup).c_str());

		std::string maxPlayerLengthStr = lang.translate(LangKey::SettingsMaxPlayerLength);
		maxPlayerLengthStr.append("##MaxPlayerLength");
		ImGui::InputInt(maxPlayerLengthStr.c_str(), &settings.tables[tableIndex].max_player_length);
		
		ProgressBarColoringMode& show_colored = settings.tables[tableIndex].show_colored;
		std::string showColoredText = lang.translate(LangKey::SettingsColoringMode);
		showColoredText.append("###ShowColored");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(showColoredText.c_str(), show_colored, ProgressBarColoringMode::LAST_ENTRY);
		ImGui::PopItemWidth();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsStyle).c_str())) {
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowProgressBar).c_str(), &settings.tables[tableIndex].show_uptime_as_progress_bar);
		ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.tables[tableIndex].alternating_row_bg);
		ImGui::Checkbox(lang.translate(LangKey::SettingsHideHeader).c_str(), &settings.tables[tableIndex].hide_header);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowOnlySubgroup).c_str(), &settings.tables[tableIndex].show_only_subgroup);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowBackground).c_str(), &settings.tables[tableIndex].show_background);
		ImGui::Checkbox(lang.translate(LangKey::SettingsScrollbar).c_str(), &settings.tables[tableIndex].scrollbar);
		ImGui::Checkbox(lang.translate(LangKey::SettingsTablePaddingX).c_str(), &settings.tables[tableIndex].table_padding_x);

		ImGui::Separator();

		Alignment& alignment = settings.tables[tableIndex].alignment;
		std::string alignmentText = lang.translate(LangKey::SettingsAlignment);
		alignmentText.append("###Alignment");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(alignmentText.c_str(), alignment, Alignment::FINAL_ENTRY);
		ImGui::PopItemWidth();

		// window padding
		std::string windowPaddingText = lang.translate(LangKey::SettingsWindowPadding);
		windowPaddingText.append("##WindowPadding");
		if (ImGui::InputFloat2(windowPaddingText.c_str(), windowPadding)) {
			std::optional<ImVec2>& settingsWindowPadding = settings.tables[tableIndex].window_padding;
			if (settingsWindowPadding) {
				settingsWindowPadding->x = windowPadding[0];
				settingsWindowPadding->y = windowPadding[1];
			} else {
				settingsWindowPadding.emplace(windowPadding[0], windowPadding[1]);
			}
		}
		// always update to current windowPadding, when windowPadding is controlled by arcdps/ImGui
		if (!settings.tables[tableIndex].window_padding) {
			const auto& padding = ImGui::GetStyle().WindowPadding;
			windowPadding[0] = padding.x;
			windowPadding[1] = padding.y;
		}

		SizingPolicy& sizingPolicy = settings.tables[tableIndex].sizing_policy;
		std::string sizingPolicyText = lang.translate(LangKey::SettingsSizingPolicy);
		sizingPolicyText.append("###SizingPolicy");
		ImGuiEx::EnumCombo(sizingPolicyText.c_str(), sizingPolicy, SizingPolicy::FINAL_ENTRY);

		if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
			ImGui::Indent(20.f);
			std::string column_width_label = lang.translate(LangKey::SettingsBoonColumnWidth);
			column_width_label.append("###BoonColumnWidth");
			ImGui::SliderFloat(column_width_label.c_str(), &settings.tables[tableIndex].boon_column_width, 20, 200);
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(lang.translate(LangKey::SettingsWidthSlideTooltip).c_str());
			}
			ImGui::Unindent(20.f);
		}

		std::string id("##singleShortcut");
		id.append(std::to_string(tableIndex));
		// use the buffer in `SettingsUIGlobal`
		ImGuiEx::KeyInput(lang.translate(static_cast<LangKey>(static_cast<size_t>(LangKey::SettingsShortcut) + tableIndex)).c_str(), id.c_str(), settingsUiGlobal.shortcut[tableIndex], 4, settings.tables[tableIndex].shortcut, lang.translate(LangKey::SettingsKeyNotSetText).c_str());

		std::string appearAsInOptionStr = lang.translate(LangKey::SettingsAppearAsInOption);
		appearAsInOptionStr.append("##appearAsInOption");
		if (ImGui::InputText(appearAsInOptionStr.c_str(), appearAsInOption, 128)) {
			settings.tables[tableIndex].appear_as_in_option = appearAsInOption;
		}

		std::string titleBarStr = lang.translate(LangKey::SettingsTitleBar);
		titleBarStr.append("##titleBar");
		if (ImGui::InputText(titleBarStr.c_str(), titleBar, 128)) {
			std::optional<std::string>& optTitleBar = settings.tables[tableIndex].title_bar;
			if (optTitleBar) {
				optTitleBar = titleBar;
			} else {
				optTitleBar.emplace(titleBar);
			}
		}
		
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsPosition).c_str())) {
		ImGuiEx::EnumRadioButton(position, Position::Manual, settings.tables[tableIndex].position);

		ImGuiEx::EnumRadioButton(position, Position::ScreenRelative, settings.tables[tableIndex].position);
		if (position == static_cast<int>(Position::ScreenRelative)) {
			ImGui::Indent(15.f);

			ImGui::PushID("cornerPositionRadioButton");
			ImGuiEx::EnumRadioButton(cornerPosition, CornerPosition::TopLeft, settings.tables[tableIndex].corner_position);
			ImGuiEx::EnumRadioButton(cornerPosition, CornerPosition::TopRight, settings.tables[tableIndex].corner_position);
			ImGuiEx::EnumRadioButton(cornerPosition, CornerPosition::BottomLeft, settings.tables[tableIndex].corner_position);
			ImGuiEx::EnumRadioButton(cornerPosition, CornerPosition::BottomRight, settings.tables[tableIndex].corner_position);
			ImGui::PopID();

			ImGui::PushItemWidth(80.f);
			ImGui::DragFloat("x", &settings.tables[tableIndex].corner_vector.x);
			ImGui::DragFloat("y", &settings.tables[tableIndex].corner_vector.y);
			ImGui::PopItemWidth();

			ImGui::Unindent(15.f);
		}

		ImGuiEx::EnumRadioButton(position, Position::WindowRelative, settings.tables[tableIndex].position);
		if (position == static_cast<int>(Position::WindowRelative)) {
			ImGui::Indent(15.f);

			ImGui::TextUnformatted(lang.translate(LangKey::SettingsFromAnchorPanelCorner).c_str());
			ImGui::PushID("anchorPanelCornerPositionRadioButton");
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::TopLeft, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::TopRight, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::BottomLeft, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::BottomRight, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGui::PopID();

			ImGui::TextUnformatted(lang.translate(LangKey::SettingsToThisPanelCorner).c_str());
			ImGui::PushID("selfPanelCornerPositionRadioButton");
			ImGuiEx::EnumRadioButton(selfPanelCornerPosition, CornerPosition::TopLeft, settings.tables[tableIndex].self_panel_corner_position);
			ImGuiEx::EnumRadioButton(selfPanelCornerPosition, CornerPosition::TopRight, settings.tables[tableIndex].self_panel_corner_position);
			ImGuiEx::EnumRadioButton(selfPanelCornerPosition, CornerPosition::BottomLeft, settings.tables[tableIndex].self_panel_corner_position);
			ImGuiEx::EnumRadioButton(selfPanelCornerPosition, CornerPosition::BottomRight, settings.tables[tableIndex].self_panel_corner_position);
			ImGui::PopID();

			ImGui::PushItemWidth(80.f);
			ImGui::DragFloat("x", &settings.tables[tableIndex].corner_vector.x);
			ImGui::DragFloat("y", &settings.tables[tableIndex].corner_vector.y);
			ImGui::PopItemWidth();

			ImGuiID shownWindowName = settings.tables[tableIndex].from_window_id;
			ImGuiWindow* selectedWindow = ImGui::FindWindowByID(shownWindowName);
			std::string selectedWindowName;
			if (selectedWindow) {
				selectedWindowName = selectedWindow->Name;
				const auto findRes = selectedWindowName.find('#');
				if (findRes != std::string::npos) {
					selectedWindowName = selectedWindowName.substr(0, findRes);
				}
			}

			if (ImGui::BeginCombo(lang.translate(LangKey::SettingsFromWindowName).c_str(), selectedWindowName.c_str())) {
				for (ImGuiWindow* window : GImGui->Windows) {
					if (!window->Hidden) {
						std::string windowName = window->Name;
						if (window->ParentWindow || window->ID == currentRootWindow->ID || windowName.find("Tooltip_") != std::string::npos
							|| windowName.find("Default") != std::string::npos) {
							continue;
						}

						const auto findRes = windowName.find('#');
						if (findRes != std::string::npos) {
							windowName = windowName.substr(0, findRes);
						}

						if (ImGui::Selectable(windowName.c_str())) {
							settings.tables[tableIndex].from_window_id = window->ID;
						}
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Unindent();
		}

		ImGui::EndMenu();
	}

	ImGui::PopStyleVar();
}

void SettingsUI::initialize(int tableIndex) {
	position = static_cast<int>(settings.tables[tableIndex].position);
	cornerPosition = static_cast<int>(settings.tables[tableIndex].corner_position);
	selfPanelCornerPosition = static_cast<int>(settings.tables[tableIndex].self_panel_corner_position);
	anchorPanelCornerPosition = static_cast<int>(settings.tables[tableIndex].anchor_panel_corner_position);
	std::to_string(settings.tables[tableIndex].shortcut).copy(settingsUiGlobal.shortcut[tableIndex], 4);
	
	char *end = appearAsInOption + sizeof(appearAsInOption);
	std::fill(appearAsInOption, end, 0);
	settings.tables[tableIndex].appear_as_in_option.copy(appearAsInOption, 128);
	if (settings.tables[tableIndex].title_bar) {
		settings.tables[tableIndex].title_bar.value().copy(titleBar, 128);
	}

	const std::optional<ImVec2>& paddingOptional = settings.getWindowPadding(tableIndex);
	if (paddingOptional) {
		const ImVec2& value = paddingOptional.value();
		windowPadding[0] = value.x;
		windowPadding[1] = value.y;
	} else {
		ImGuiStyle& style = ImGui::GetStyle();
		const auto& padding = style.WindowPadding;
		windowPadding[0] = padding.x;
		windowPadding[1] = padding.y;
	}
}

bool SettingsUI::tableColumnSubMenu(Table::ImGuiTable* table, const char* label, BoonType type, int beginId) const {
	if (ImGui::BeginMenu(label)) {
		for (const BoonDef& trackedBuff : tracked_buffs) {
			if (trackedBuff.category == type) {
				Table::MenuItemTableColumnVisibility(table, beginId);
			}
			++beginId;
		}

		return true;
	}

	return false;
}

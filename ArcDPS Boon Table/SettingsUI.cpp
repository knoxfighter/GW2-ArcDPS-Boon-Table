#include "SettingsUI.h"

#include "AppChart.h"
#include "Helpers.h"
#include "History.h"
#include "Lang.h"
#include "Settings.h"
#include "SettingsUIGlobal.h"

#include <ArcdpsExtension/ExtensionTranslations.h>
#include <ArcdpsExtension/Widgets.h>
#include <magic_enum/magic_enum.hpp>

SettingsUI settingsUi;

using ArcdpsExtension::Localization;

namespace Table = ImGuiEx::BigTable;

void SettingsUI::Draw(Table::ImGuiTable* table, int tableIndex, ImGuiWindow* currentRootWindow) {
	if (ImGui::IsWindowAppearing()) {
		initialize(tableIndex);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

	if (ImGui::BeginMenu(Localization::STranslate(BT_SettingsHistory).data())) {
		ImVec4* arc_colors[5];
		arc_export_e5(arc_colors);
		
		if (settings.tables[tableIndex].current_history != 0) {
			ImGui::PushStyleColor(ImGuiCol_Text, arc_colors[0][CCOL_LGREY]);
		}
		ImGui::TextUnformatted(Localization::STranslate(BT_SettingsHistoryCurrent).data());
		if (settings.tables[tableIndex].current_history != 0) {
			ImGui::PopStyleColor();
		}
		if (ImGui::IsItemClicked()) {
			settings.tables[tableIndex].current_history = 0;
		}

		std::unique_lock<std::mutex> guard = history.lock();
		// 1-based, 0 = current
		int historyIndex = 1;
		for (const TrackerHistory& trackerHistory : history) {
			auto duration = trackerHistory.getDuration();

			using namespace std::chrono;

			// starttime: <hh>:<mm>:<ss>, <mm>m <ss>s (<name>)
			auto starttime = time_point_cast<seconds>(trackerHistory.getStarttime());
			auto starttime_t = system_clock::to_time_t(starttime);

			// convert starttime to local and replace if successful
			tm tm;
			if (localtime_s(&tm, &starttime_t) == S_OK) {
				starttime = time_point_cast<seconds>(system_clock::time_point(hours(tm.tm_hour) + minutes(tm.tm_min) + seconds(tm.tm_sec)));
			}


			auto durationMinutes = duration >= minutes(1) ? std::format(" {:%M}m", duration) : "";
			auto str = std::format("{:%T},{} {:%S}s ({})", starttime, durationMinutes, duration_cast<seconds>(duration), trackerHistory.getLogName());

			if (settings.tables[tableIndex].current_history != historyIndex) {
				ImGui::PushStyleColor(ImGuiCol_Text, arc_colors[0][CCOL_LGREY]);
			}
			ImGui::TextUnformatted(str.c_str());
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

	ImGui::Checkbox(Localization::STranslate(BT_SettingsSelfOnTop).data(), &settings.tables[tableIndex].show_self_on_top);
	ImGui::Checkbox(Localization::STranslate(BT_SettingsPlayers).data(), &settings.tables[tableIndex].show_players);
	ImGui::Checkbox(Localization::STranslate(BT_SettingsSubgroups).data(), &settings.tables[tableIndex].show_subgroups);
	ImGui::Checkbox(Localization::STranslate(BT_SettingsTotal).data(), &settings.tables[tableIndex].show_total);

	ImGui::Separator();
	if (ImGui::BeginMenu(Localization::STranslate(ArcdpsExtension::ET_ColumnSetup).data(), table != nullptr)) {
		ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

		// username
		Table::MenuItemTableColumnVisibility(table, 0);
		// subgroup
		Table::MenuItemTableColumnVisibility(table, 1);

		// Submenus for controlling visibility
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeBoon).data(), BoonType_boon, 2))
			ImGui::EndMenu();
		// if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeWarrior).data(), BoonType_Warrior, 2))
		// 	ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeRevenant).data(), BoonType_Revenant, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeGuardian).data(), BoonType_Guardian, 2))
			ImGui::EndMenu();
		// if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeEngineer).data(), BoonType_Engineer, 2))
		// 	ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeRanger).data(), BoonType_Ranger, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeEngineer).data(), BoonType_Elementalist, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeMesmer).data(), BoonType_Mesmer, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeNecromancer).data(), BoonType_Necromancer, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeAura).data(), BoonType_Aura, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeRelic).data(), BoonType_Relic, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, Localization::STranslate(BT_BoonTypeOther).data(), BoonType_other, 2)) {
			Table::MenuItemTableColumnVisibility(table, tracked_buffs.size() + 2);
			ImGui::EndMenu();
		}

		if (ImGui::Button(Localization::STranslate(BT_SettingsResetTableColumns).data())) {
			Table::TableResetSettings(table);
		}

		ImGui::PopItemFlag();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(Localization::STranslate(BT_SettingsDisplay).data())) {
		ImGui::Checkbox(Localization::STranslate(BT_SettingsShowLabel).data(), &settings.tables[tableIndex].show_label);

		std::string_view maxDisplayedInputLabel = Localization::STranslate(ArcdpsExtension::ET_MaxDisplayed);
		ImGui::InputInt(std::format("{}###maxDisplayedInput", maxDisplayedInputLabel).c_str(), &settings.tables[tableIndex].max_displayed);
		ImGui::SameLine();
		HelpMarker(Localization::STranslate(BT_SettingsMaxDisplayedPopup).data());

		std::string_view maxPlayerLengthStr = Localization::STranslate(BT_SettingsMaxPlayerLength);
		ImGui::InputInt(std::format("{}###maxPlayerLength", maxPlayerLengthStr).c_str(), &settings.tables[tableIndex].max_player_length);
		
		ProgressBarColoringMode& show_colored = settings.tables[tableIndex].show_colored;
		std::string_view showColoredText = Localization::STranslate(BT_SettingsColoringMode);
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(std::format("{}###ShowColored", showColoredText).c_str(), show_colored, ProgressBarColoringMode::LAST_ENTRY);
		ImGui::PopItemWidth();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(Localization::STranslate(ArcdpsExtension::ET_Style).data())) {
		ImGui::Checkbox(Localization::STranslate(BT_SettingsShowProgressBar).data(), &settings.tables[tableIndex].show_uptime_as_progress_bar);
		ImGui::Checkbox(Localization::STranslate(ArcdpsExtension::ET_AlternatingRowBg).data(), &settings.tables[tableIndex].alternating_row_bg);
		
		// We use the text "Show Header" here, which means we have to negate the settings-value
		bool hide_header_tmp = !settings.tables[tableIndex].hide_header;
		if (ImGui::Checkbox(Localization::STranslate(ArcdpsExtension::ExtensionTranslation::ET_TitleBar).data(), &hide_header_tmp)) {
			settings.tables[tableIndex].hide_header = !hide_header_tmp;
		}
		ImGui::Checkbox(Localization::STranslate(BT_SettingsShowOnlySubgroup).data(), &settings.tables[tableIndex].show_only_subgroup);
		ImGui::Checkbox(Localization::STranslate(ArcdpsExtension::ET_Background).data(), &settings.tables[tableIndex].show_background);
		ImGui::Checkbox(Localization::STranslate(ArcdpsExtension::ET_Scrollbar).data(), &settings.tables[tableIndex].scrollbar);
		ImGui::Checkbox(Localization::STranslate(BT_SettingsTablePaddingX).data(), &settings.tables[tableIndex].table_padding_x);

		ImGui::Separator();

		Alignment& alignment = settings.tables[tableIndex].alignment;
		std::string_view alignmentText = Localization::STranslate(BT_SettingsAlignment);
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(std::format("{}###Alignment", alignmentText).c_str(), alignment, magic_enum::enum_values<Alignment>());
		ImGui::PopItemWidth();

		// window padding
		std::string_view windowPaddingText = Localization::STranslate(ArcdpsExtension::ET_Padding);
		if (ImGui::DragFloat2(std::format("{}###WindowPadding", windowPaddingText).c_str(), windowPadding)) {
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
		std::string_view sizingPolicyText = Localization::STranslate(ArcdpsExtension::ET_SizingPolicy);
		ImGuiEx::EnumCombo(std::format("{}###SizingPolicy", sizingPolicyText).c_str(), sizingPolicy, magic_enum::enum_values<SizingPolicy>());

		if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
			ImGui::Indent(20.f);
			std::string_view column_width_label = Localization::STranslate(BT_SettingsBoonColumnWidth);
			ImGui::SliderFloat(std::format("{}###BoonColumnWidth", column_width_label).c_str(), &settings.tables[tableIndex].boon_column_width, 20, 200);
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", Localization::STranslate(BT_SettingsWidthSlideTooltip).data());
			}
			ImGui::Unindent(20.f);
		}

		std::string id("##singleShortcut");
		id.append(std::to_string(tableIndex));
		// use the buffer in `SettingsUIGlobal`
		ImGuiEx::KeyInput(
			std::format("{} #{}", Localization::STranslate(ArcdpsExtension::ET_Shortcut), tableIndex).c_str(),
			id.c_str(),
			settingsUiGlobal.shortcut[tableIndex],
			4,
			settings.tables[tableIndex].shortcut,
			Localization::STranslate(BT_SettingsKeyNotSetText).data()
		);

		std::string_view appearAsInOptionStr = Localization::STranslate(ArcdpsExtension::ET_AppearAsInOption);
		if (ImGui::InputText(std::format("{}###appearAsInOption", appearAsInOptionStr).c_str(), appearAsInOption, 128)) {
			settings.tables[tableIndex].appear_as_in_option = appearAsInOption;
		}

		std::string_view titleBarStr = Localization::STranslate(ArcdpsExtension::ET_TitleBar);
		if (ImGui::InputText(std::format("{}###titleBar", titleBarStr).c_str(), titleBar, 128)) {
			std::optional<std::string>& optTitleBar = settings.tables[tableIndex].title_bar;
			if (optTitleBar) {
				optTitleBar = titleBar;
			} else {
				optTitleBar.emplace(titleBar);
			}
		}
		
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(Localization::STranslate(ArcdpsExtension::ET_Position).data())) {
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

			ImGui::TextUnformatted(Localization::STranslate(ArcdpsExtension::ET_FromAnchorPanelCorner).data());
			ImGui::PushID("anchorPanelCornerPositionRadioButton");
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::TopLeft, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::TopRight, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::BottomLeft, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGuiEx::EnumRadioButton(anchorPanelCornerPosition, CornerPosition::BottomRight, settings.tables[tableIndex].anchor_panel_corner_position);
			ImGui::PopID();

			ImGui::TextUnformatted(Localization::STranslate(ArcdpsExtension::ET_ThisPanelCorner).data());
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

			if (ImGui::BeginCombo(Localization::STranslate(ArcdpsExtension::ET_AnchorWindow).data(), selectedWindowName.c_str())) {
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

#include "SettingsUI.h"

#include "Helpers.h"
#include "Lang.h"
#include "Settings.h"
#include "extension/Widgets.h"

SettingsUI settingsUi;

namespace Table = ImGuiEx::BigTable;

void SettingsUI::Draw(Table::ImGuiTable* table, int tableIndex, ImGuiWindow* currentRootWindow) {
	if (ImGui::IsWindowAppearing()) {
		initialize(tableIndex);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

	ImGui::Checkbox(lang.translate(LangKey::SettingsSelfOnTop).c_str(), &settings.tables[tableIndex].show_self_on_top);
	ImGui::Checkbox(lang.translate(LangKey::SettingsPlayers).c_str(), &settings.tables[tableIndex].show_players);
	ImGui::Checkbox(lang.translate(LangKey::SettingsSubgroups).c_str(), &settings.tables[tableIndex].show_subgroups);
	ImGui::Checkbox(lang.translate(LangKey::SettingsTotal).c_str(), &settings.tables[tableIndex].show_total);
	ImGui::Checkbox(lang.translate(LangKey::SettingsNPC).c_str(), &settings.tables[tableIndex].show_npcs);

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
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeWarrior).c_str(), BoonType_Warrior, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeRevenant).c_str(), BoonType_Revenant, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeGuardian).c_str(), BoonType_Guardian, 2))
			ImGui::EndMenu();
		if (tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeEngineer).c_str(), BoonType_Engineer, 2))
			ImGui::EndMenu();
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

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsStyle).c_str())) {
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowProgressBar).c_str(), &settings.tables[tableIndex].show_uptime_as_progress_bar);
		ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.tables[tableIndex].alternating_row_bg);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.tables[tableIndex].show_label);
		ImGui::Checkbox(lang.translate(LangKey::SettingsHideHeader).c_str(), &settings.tables[tableIndex].hide_header);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowOnlySubgroup).c_str(), &settings.tables[tableIndex].show_only_subgroup);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowBackground).c_str(), &settings.tables[tableIndex].show_background);

		ProgressBarColoringMode& show_colored = settings.tables[tableIndex].show_colored;
		std::string showColoredText = lang.translate(LangKey::SettingsColoringMode);
		showColoredText.append("###ShowColored");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(showColoredText.c_str(), show_colored, ProgressBarColoringMode::LAST_ENTRY);
		ImGui::PopItemWidth();

		if (show_colored == ProgressBarColoringMode::ByPercentage) {
			ImGui::Indent(20.f);
			if (ImGui::ColorEdit4(lang.translate(LangKey::Settings100Color).c_str(), _100color)) {
				// i think the color changed
				if (settings._100_color) {
					settings._100_color->x = _100color[0];
					settings._100_color->y = _100color[1];
					settings._100_color->z = _100color[2];
					settings._100_color->w = _100color[3];
				}
				else {
					settings._100_color = ImVec4(_100color[0], _100color[1], _100color[2], _100color[3]);
				}
			}
			if (ImGui::ColorEdit4(lang.translate(LangKey::Settings0Color).c_str(), _0color)) {
				// i think the color changed
				if (settings._0_color) {
					settings._0_color->x = _0color[0];
					settings._0_color->y = _0color[1];
					settings._0_color->z = _0color[2];
					settings._0_color->w = _0color[3];
				}
				else {
					settings._0_color = ImVec4(_0color[0], _0color[1], _0color[2], _0color[3]);
				}
			}
			ImGui::Unindent(20.f);
		}

		Alignment& alignment = settings.tables[tableIndex].alignment;
		std::string alignmentText = lang.translate(LangKey::SettingsAlignment);
		alignmentText.append("###Alignment");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(alignmentText.c_str(), alignment, Alignment::FINAL_ENTRY);
		ImGui::PopItemWidth();

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

		if (ImGui::ColorEdit4(lang.translate(LangKey::SettingsSelfColor).c_str(), self_color)) {
			// i think the color changed
			if (settings.self_color) {
				settings.self_color->x = self_color[0];
				settings.self_color->y = self_color[1];
				settings.self_color->z = self_color[2];
				settings.self_color->w = self_color[3];
			}
			else {
				settings.self_color = ImVec4(self_color[0], self_color[1], self_color[2], self_color[3]);
			}
		}
		// always update to current selfColor, when selfColor is controlled by arcdps
		if (!settings.self_color) {
			const ImVec4& imVec4 = settings.getSelfColor();
			self_color[0] = imVec4.x;
			self_color[1] = imVec4.y;
			self_color[2] = imVec4.z;
			self_color[3] = imVec4.w;
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
							|| windowName.find("Default") != std::string::npos || windowName.find('/') != std::string::npos) {
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
	const ImVec4& imVec4 = settings.getSelfColor();
	self_color[0] = imVec4.x;
	self_color[1] = imVec4.y;
	self_color[2] = imVec4.z;
	self_color[3] = imVec4.w;

	const ImVec4& imVec4_2 = settings.get100Color();
	_100color[0] = imVec4_2.x;
	_100color[1] = imVec4_2.y;
	_100color[2] = imVec4_2.z;
	_100color[3] = imVec4_2.w;

	const ImVec4& imVec4_3 = settings.get0Color();
	_0color[0] = imVec4_3.x;
	_0color[1] = imVec4_3.y;
	_0color[2] = imVec4_3.z;
	_0color[3] = imVec4_3.w;

	position = static_cast<int>(settings.tables[tableIndex].position);
	cornerPosition = static_cast<int>(settings.tables[tableIndex].corner_position);
	selfPanelCornerPosition = static_cast<int>(settings.tables[tableIndex].self_panel_corner_position);
	anchorPanelCornerPosition = static_cast<int>(settings.tables[tableIndex].anchor_panel_corner_position);
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

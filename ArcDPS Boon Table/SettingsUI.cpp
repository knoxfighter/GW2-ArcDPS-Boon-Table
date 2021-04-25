#include "SettingsUI.h"

#include "Helpers.h"
#include "Lang.h"
#include "Settings.h"
#include "extension/Widgets.h"

SettingsUI settingsUi;

void SettingsUI::Draw(ImGuiTable* table, int tableIndex) {
	if (!init) {
		init = true;
		initialize();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

	ImGui::Checkbox(lang.translate(LangKey::SettingsPlayers).c_str(), &settings.tables[tableIndex].show_players);
	ImGui::Checkbox(lang.translate(LangKey::SettingsSubgroups).c_str(), &settings.tables[tableIndex].show_subgroups);
	ImGui::Checkbox(lang.translate(LangKey::SettingsTotal).c_str(), &settings.tables[tableIndex].show_total);
	ImGui::Checkbox(lang.translate(LangKey::SettingsNPC).c_str(), &settings.tables[tableIndex].show_npcs);

	ImGui::Separator();
	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsColumnSetup).c_str(), table != nullptr)) {
		ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

		// username
		ImGuiEx::MenuItemTableColumnVisibility(table, 0);
		// subgroup
		ImGuiEx::MenuItemTableColumnVisibility(table, 1);

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
			ImGuiEx::MenuItemTableColumnVisibility(table, tracked_buffs.size() + 2);
			ImGui::EndMenu();
		}

		ImGui::PopItemFlag();

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu(lang.translate(LangKey::SettingsStyle).c_str())) {
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowProgressBar).c_str(), &settings.tables[tableIndex].show_boon_as_progress_bar);
		ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.tables[tableIndex].alternating_row_bg);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.tables[tableIndex].show_label);
		ImGui::Checkbox(lang.translate(LangKey::SettingsHideHeader).c_str(), &settings.tables[tableIndex].hide_header);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowOnlySubgroup).c_str(), &settings.tables[tableIndex].show_only_subgroup);
		ImGui::Checkbox(lang.translate(LangKey::SettingsShowOnlySelf).c_str(), &settings.tables[tableIndex].show_only_self);

		ProgressBarColoringMode& show_colored = settings.tables[tableIndex].show_colored;
		std::string showColoredText = lang.translate(LangKey::SettingsColoringMode);
		showColoredText.append("###ShowColored");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(showColoredText.c_str(), show_colored, ProgressBarColoringMode::LAST_ENTRY);
		ImGui::PopItemWidth();

		Alignment& alignment = settings.tables[tableIndex].alignment;
		std::string alignmentText = lang.translate(LangKey::SettingsAlignment);
		alignmentText.append("###Alignment");
		ImGui::PushItemWidth(120);
		ImGuiEx::EnumCombo(alignmentText.c_str(), alignment, Alignment::FINAL_ENTRY);
		ImGui::PopItemWidth();

		SizingPolicy& sizingPolicy = settings.tables[tableIndex].sizingPolicy;
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
	

	ImGui::PopStyleVar();
}

void SettingsUI::initialize() {
	const ImVec4& imVec4 = settings.getSelfColor();
	self_color[0] = imVec4.x;
	self_color[1] = imVec4.y;
	self_color[2] = imVec4.z;
	self_color[3] = imVec4.w;
}

bool SettingsUI::tableColumnSubMenu(ImGuiTable* table, const char* label, BoonType type, int beginId) const {
	if (ImGui::BeginMenu(label)) {
		for (const BoonDef& trackedBuff : tracked_buffs) {
			if (trackedBuff.category == type) {
				ImGuiEx::MenuItemTableColumnVisibility(table, beginId);
			}
			++beginId;
		}

		return true;
	}

	return false;
}

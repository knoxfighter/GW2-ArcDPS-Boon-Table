#include "SettingsUI.h"

#include "Helpers.h"
#include "Lang.h"
#include "Settings.h"
#include "extension/Widgets.h"

SettingsUI settingsUi;

void SettingsUI::Draw(ImGuiTable* table) {
	if (!init) {
		init = true;
		initialize();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

	if (table) {
		if (ImGui::BeginMenu(lang.translate(LangKey::SettingsColumnSetup).c_str())) {
			ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

			// username
			ImGuiEx::MenuItemTableColumnVisibility(table, 0);
			// subgroup
			ImGuiEx::MenuItemTableColumnVisibility(table, 1);

			// Submenus for controlling visibility
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeBoon).c_str(), BoonType_boon);
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeTrait).c_str(), BoonType_trait);
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeBanner).c_str(), BoonType_banner);
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeSprit).c_str(), BoonType_spirit);
			// tableColumnSubMenu(imGuiTable, lang.translate(LangKey::BoonTypeSkill).c_str(),", BoonType_skill); // this boontype is empty
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeSignet).c_str(), BoonType_signet);
			tableColumnSubMenu(table, lang.translate(LangKey::BoonTypeOther).c_str(), BoonType_other);

			ImGui::PopItemFlag();

			ImGui::EndMenu();
		}
	}

	ImGui::Separator();

	ImGui::Checkbox(lang.translate(LangKey::SettingsPlayers).c_str(), &settings.show_players);
	ImGui::Checkbox(lang.translate(LangKey::SettingsSubgroups).c_str(), &settings.show_subgroups);
	ImGui::Checkbox(lang.translate(LangKey::SettingsTotal).c_str(), &settings.show_total);
	ImGui::Checkbox(lang.translate(LangKey::SettingsNPC).c_str(), &settings.show_npcs);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowProgressBar).c_str(), &settings.show_boon_as_progress_bar);
	ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.alternating_row_bg);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.show_label);
	ImGui::Checkbox(lang.translate(LangKey::SettingsHideHeader).c_str(), &settings.hide_header);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowOnlySubgroup).c_str(), &settings.show_only_subgroup);

	ProgressBarColoringMode& show_colored = settings.show_colored;
	std::string showColoredText = lang.translate(LangKey::SettingsColoringMode);
	showColoredText.append("###ShowColored");
	ImGui::PushItemWidth(120);
	if (ImGui::BeginCombo(showColoredText.c_str(), to_string(show_colored).c_str())) {
		ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::Uncolored);
		ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::ByProfession);
		ImGuiEx::Selectable(show_colored, ProgressBarColoringMode::ByPercentage);

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	Alignment& alignment = settings.alignment;
	std::string alignmentText = lang.translate(LangKey::SettingsAlignment);
	alignmentText.append("###Alignment");
	ImGui::PushItemWidth(120);
	if (ImGui::BeginCombo(alignmentText.c_str(), to_string(alignment).c_str())) {
		ImGuiEx::Selectable(alignment, Alignment::Unaligned);
		ImGuiEx::Selectable(alignment, Alignment::Left);
		ImGuiEx::Selectable(alignment, Alignment::Center);
		ImGuiEx::Selectable(alignment, Alignment::Right);

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	SizingPolicy& sizingPolicy = settings.sizingPolicy;
	std::string sizingPolicyText = lang.translate(LangKey::SettingsSizingPolicy);
	sizingPolicyText.append("###SizingPolicy");
	if (ImGui::BeginCombo(sizingPolicyText.c_str(), to_string(sizingPolicy).c_str())) {
		ImGuiEx::Selectable(sizingPolicy, SizingPolicy::SizeContentToWindow);
		ImGuiEx::Selectable(sizingPolicy, SizingPolicy::ManualWindowSize);
		ImGuiEx::Selectable(sizingPolicy, SizingPolicy::SizeToContent);

		ImGui::EndCombo();
	}

	ImGui::Separator();
	
	if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
		ImGui::Indent(20.f);
		std::string column_width_label = lang.translate(LangKey::SettingsBoonColumnWidth);
		column_width_label.append("###BoonColumnWidth");
		ImGui::SliderFloat(column_width_label.c_str(), &settings.boon_column_width, 20, 200);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(lang.translate(LangKey::SettingsWidthSlideTooltip).c_str());
		}
		ImGui::Unindent(20.f);
	}

	ImGui::Separator();

	// ImGui::ColorPicker4()
	if (ImGui::ColorEdit4(lang.translate(LangKey::SettingsSelfColor).c_str(), self_color)) {
		// i think the color changed
		if (settings.self_color) {
			settings.self_color->x = self_color[0];
			settings.self_color->y = self_color[1];
			settings.self_color->z = self_color[2];
			settings.self_color->w = self_color[3];
		} else {
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

	ImGui::PopStyleVar();
}

void SettingsUI::initialize() {
	const ImVec4& imVec4 = settings.getSelfColor();
	self_color[0] = imVec4.x;
	self_color[1] = imVec4.y;
	self_color[2] = imVec4.z;
	self_color[3] = imVec4.w;
}

void SettingsUI::tableColumnSubMenu(ImGuiTable* table, const char* label, BoonType type) const {
	if (ImGui::BeginMenu(label)) {
		int i = 2;
		for (const BoonDef& trackedBuff : tracked_buffs) {
			if (trackedBuff.category == type) {
				ImGuiEx::MenuItemTableColumnVisibility(table, i);
			}
			++i;
		}

		ImGui::EndMenu();
	}
}

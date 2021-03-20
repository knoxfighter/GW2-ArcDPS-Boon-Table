#include "SettingsUI.h"

#include "Helpers.h"
#include "Lang.h"
#include "Settings.h"
#include "extension/Widgets.h"
#include "imgui/imgui.h"

void SettingsUI::Draw() {
	ImGui::Checkbox(lang.translate(LangKey::SettingsPlayers).c_str(), &settings.show_players);
	ImGui::Checkbox(lang.translate(LangKey::SettingsSubgroups).c_str(), &settings.show_subgroups);
	ImGui::Checkbox(lang.translate(LangKey::SettingsTotal).c_str(), &settings.show_total);
	ImGui::Checkbox(lang.translate(LangKey::SettingsNPC).c_str(), &settings.show_npcs);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowProgressBar).c_str(), &settings.show_boon_as_progress_bar);
	ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.alternating_row_bg);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.show_label);
	ImGui::Checkbox(lang.translate(LangKey::SettingsHideHeader).c_str(), &settings.hide_header);

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
	if (sizingPolicy == SizingPolicy::SizeToContent || sizingPolicy == SizingPolicy::ManualWindowSize) {
		ImGui::Indent(20.f);
		std::string column_width_label = lang.translate(LangKey::SettingsBoonColumnWidth);
		column_width_label.append("###BoonColumnWidth");
		ImGui::SliderFloat(column_width_label.c_str(), &settings.boon_column_width, 20, 200);
	}
}

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
	ImGui::Checkbox(lang.translate(LangKey::SettingsAlwaysResize).c_str(), &settings.size_to_content);
	ImGui::Checkbox(lang.translate(LangKey::SettingsAlternatingRow).c_str(), &settings.alternating_row_bg);
	ImGui::Checkbox(lang.translate(LangKey::SettingsShowLabel).c_str(), &settings.show_label);

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
}

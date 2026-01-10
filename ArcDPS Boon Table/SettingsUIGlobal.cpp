#include "SettingsUIGlobal.h"

// (╯°□°）╯︵ ┻━┻
// this includes are needed for some absolutely unknown reason...
#include <optional>
#include <array>
#include <charconv>
// ┬─┬ ノ( ゜-゜ノ)

#include "Lang.h"
#include <imgui/imgui.h>
#include <ArcdpsExtension/Widgets.h>

SettingsUIGlobal settingsUiGlobal;

template<typename T>
bool OptionalSetting(std::optional<T>& setting, const char* title, const char* checkboxId, std::function<T()> constructValue, std::function<void()> children) {
	ImGui::TextUnformatted(title);
	ImGui::SameLine();
	bool settingActive = setting.has_value();
	
	if (ImGui::Checkbox(checkboxId, &settingActive)) {
		if (settingActive) {
			setting = constructValue();
		} else {
			setting.reset();
		}
	}
	ImGui::SameLine();

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !settingActive);

	children();

	// Pop ImGuiItemFlags_Disabled
	ImGui::PopItemFlag();

	return false;
}

void SettingsUIGlobal::Draw() {
	if (!initialized) {
		initialize();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.f, 0.f});

	ImGui::BeginTable("###SettingsGeneralTable", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedFit);

	ImGui::TableSetupColumn("0");
	ImGui::TableSetupColumn("1");

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	ImGui::TextDisabled("General");

	// fights to keep
	ImGui::InputInt(lang.translate(LangKey::SettingsFightsToKeep).c_str(), &settings.fights_to_keep);

	OptionalSetting<ImVec4>(settings._100_color, lang.translate(LangKey::Settings100Color).c_str(), "###100colorCheckbox", []{return settings.get100Color();}, [] {
		auto color = settings.get100Color();
		if (ImGui::ColorEdit4("###100colorEdit", &color.x)) {
			settings._100_color = color;
		}
	});
	
	OptionalSetting<ImVec4>(settings._0_color, lang.translate(LangKey::Settings100Color).c_str(), "###0colorCheckbox", []{return settings.get0Color();}, [] {
		auto color = settings.get0Color();
		if (ImGui::ColorEdit4("###0colorEdit", &color.x)) {
			settings._0_color = color;
		}
	});

	OptionalSetting<ImVec4>(settings.self_color, lang.translate(LangKey::SettingsSelfColor).c_str(), "###self_color_checkbox", [] {return settings.getSelfColor();}, [&] {
		auto color = settings.getSelfColor();
		if (ImGui::ColorEdit4("###self_color_edit", &color.x)) {
			settings.self_color = color;
		}
	});

	ImGui::TableNextColumn();

	ImGui::TextDisabled("Shortcuts");

	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		std::string id("##shortcut");
		id.append(std::to_string(i));
		ImGuiEx::KeyInput(lang.translate(static_cast<LangKey>(static_cast<size_t>(LangKey::SettingsShortcut) + i)).c_str(), id.c_str(), shortcut[i], 4, settings.tables[i].shortcut, lang.translate(LangKey::SettingsKeyNotSetText).c_str());
	}

	ImGui::EndTable();

	ImGui::PopStyleVar();
}

void SettingsUIGlobal::initialize() {
	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		if (settings.tables[i].shortcut != 0) {
			std::to_string(settings.tables[i].shortcut).copy(shortcut[i], 4);
		}
	}

	initialized = true;
}

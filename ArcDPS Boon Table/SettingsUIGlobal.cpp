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

using ArcdpsExtension::Localization;

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

	auto& localization = Localization::instance();

	ImGui::TextDisabled("%s", localization.Translate(BT_SettingsGeneral).data());

	// fights to keep
	ImGui::InputInt(localization.Translate(BT_SettingsFightsToKeep).data(), &settings.fights_to_keep);

	ImGuiEx::OptionalSetting<ImVec4>(settings._100_color, localization.Translate(BT_Settings100Color).data(), "###100colorCheckbox", []{return settings.get100Color();}, [] {
		auto color = settings.get100Color();
		if (ImGui::ColorEdit4("###100colorEdit", &color.x)) {
			settings._100_color = color;
		}
	});

	ImGuiEx::OptionalSetting<ImVec4>(settings._0_color, localization.Translate(BT_Settings0Color).data(), "###0colorCheckbox", []{return settings.get0Color();}, [] {
		auto color = settings.get0Color();
		if (ImGui::ColorEdit4("###0colorEdit", &color.x)) {
			settings._0_color = color;
		}
	});

	ImGuiEx::OptionalSetting<ImVec4>(settings.self_color, localization.Translate(BT_SettingsSelfColor).data(), "###self_color_checkbox", [] {return settings.getSelfColor();}, [&] {
		auto color = settings.getSelfColor();
		if (ImGui::ColorEdit4("###self_color_edit", &color.x)) {
			settings.self_color = color;
		}
	});

	auto langLabel = std::format("{}###boontable_language", localization.Translate(ArcdpsExtension::ET_Language));
	auto langLabelPreview = settings.language2 == Lang::LikeGame ? localization.Translate(ArcdpsExtension::ET_LikeInGame) : localization.Translate(settings.language2, ArcdpsExtension::ET_LanguageName);
	if (ImGui::BeginCombo(langLabel.c_str(), langLabelPreview.data())) {
		for (auto& language : localization.GetLanguages()) {
			if (ImGui::Selectable(localization.Translate(language, ArcdpsExtension::ET_LanguageName).data(), language == settings.language2)) {
				settings.setLanguage(language);
			}
		}
		if (ImGui::Selectable(localization.Translate(ArcdpsExtension::ET_LikeInGame).data())) {
			settings.setLanguage(Lang::LikeGame);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", Localization::STranslate(BT_LikeInGameUEOnlyTooltip).data());
		}

		ImGui::EndCombo();
	}

	ImGui::TableNextColumn();

	ImGui::TextDisabled("%s", localization.Translate(BT_SettingsGlobalShortcuts).data());

	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		std::string id("##shortcut");
		id.append(std::to_string(i));
		ImGuiEx::KeyInput(
			std::format("{} #{}", localization.Translate(BT_SettingsShortcut), i).c_str(), 
			id.c_str(), 
			shortcut[i], 
			4, 
			settings.tables[i].shortcut, 
			localization.Translate(BT_SettingsKeyNotSetText).data()
		);
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

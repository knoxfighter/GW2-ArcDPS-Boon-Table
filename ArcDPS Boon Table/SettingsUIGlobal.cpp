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

	ImGui::TextDisabled("%s", Localization::STranslate(BT_SettingsGeneral).data());

	// fights to keep
	ImGui::InputInt(Localization::STranslate(BT_SettingsFightsToKeep).data(), &settings.fights_to_keep);

	ImGuiEx::OptionalSetting<ImVec4>(settings._100_color, Localization::STranslate(BT_Settings100Color).data(), "###100colorCheckbox", []{return settings.get100Color();}, [] {
		auto color = settings.get100Color();
		if (ImGui::ColorEdit4("###100colorEdit", &color.x)) {
			settings._100_color = color;
		}
	});

	ImGuiEx::OptionalSetting<ImVec4>(settings._0_color, Localization::STranslate(BT_Settings0Color).data(), "###0colorCheckbox", []{return settings.get0Color();}, [] {
		auto color = settings.get0Color();
		if (ImGui::ColorEdit4("###0colorEdit", &color.x)) {
			settings._0_color = color;
		}
	});

	ImGuiEx::OptionalSetting<ImVec4>(settings.self_color, Localization::STranslate(BT_SettingsSelfColor).data(), "###self_color_checkbox", [] {return settings.getSelfColor();}, [&] {
		auto color = settings.getSelfColor();
		if (ImGui::ColorEdit4("###self_color_edit", &color.x)) {
			settings.self_color = color;
		}
	});

	auto langLabel = std::format("{}###boontable_language", Localization::STranslate(ArcdpsExtension::ET_Language));
	using ArcdpsExtension::LanguageSetting;
	static const std::map<LanguageSetting, std::function<std::string()>> tooltips = {
		{LanguageSetting::LikeGame, [] {
			return std::string(Localization::STranslate(BT_LikeInGameUEOnlyTooltip));
		}}
	};
	if (ImGuiEx::EnumCombo(
		langLabel.c_str(), 
		settings.language,
		{LanguageSetting::English, LanguageSetting::German, LanguageSetting::French, LanguageSetting::Spanish, LanguageSetting::Chinese, LanguageSetting::LikeGame},
		tooltips
	)) {
		auto lang = settings.language;
		if (settings.language == LanguageSetting::LikeGame) {
			lang = settings.gameLanguage;
		}
		Localization::instance().ChangeLanguage(static_cast<gwlanguage>(lang));
	}

	ImGui::TableNextColumn();

	ImGui::TextDisabled("%s", Localization::STranslate(BT_SettingsGlobalShortcuts).data());

	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		std::string id("##shortcut");
		id.append(std::to_string(i));
		ImGuiEx::KeyInput(
			std::format("{} #{}", Localization::STranslate(BT_SettingsShortcut), i).c_str(), 
			id.c_str(), 
			shortcut[i], 
			4, 
			settings.tables[i].shortcut, 
			Localization::STranslate(BT_SettingsKeyNotSetText).data()
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

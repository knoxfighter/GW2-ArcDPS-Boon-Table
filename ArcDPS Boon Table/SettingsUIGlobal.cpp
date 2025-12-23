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

	if (ImGui::ColorEdit4(lang.translate(LangKey::Settings100Color).c_str(), _100color)) {
		// i think the color changed
		if (settings._100_color) {
			settings._100_color->x = _100color[0];
			settings._100_color->y = _100color[1];
			settings._100_color->z = _100color[2];
			settings._100_color->w = _100color[3];
		} else {
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
		} else {
			settings._0_color = ImVec4(_0color[0], _0color[1], _0color[2], _0color[3]);
		}
	}

	if (ImGui::ColorEdit4(lang.translate(LangKey::SettingsSelfColor).c_str(), self_color)) {
		// color changed
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

	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		if (settings.tables[i].shortcut != 0) {
			std::to_string(settings.tables[i].shortcut).copy(shortcut[i], 4);
		}
	}

	initialized = true;
}

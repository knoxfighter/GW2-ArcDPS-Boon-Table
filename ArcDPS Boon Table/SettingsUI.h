#pragma once

#include "BuffIds.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

class SettingsUI {
public:
	void Draw(ImGuiTable* table);

private:
	float self_color[4]{};
	bool init = false;
	
	void initialize();
	void tableColumnSubMenu(ImGuiTable* table, const char* label, BoonType type) const;
};

extern SettingsUI settingsUi;

#pragma once

#include "BuffIds.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

class SettingsUI {
public:
	void Draw(ImGuiTable* table, int tableIndex);

private:
	float self_color[4]{};
	bool init = false;
	
	void initialize();
	bool tableColumnSubMenu(ImGuiTable* table, const char* label, BoonType type, int beginId) const;
};

extern SettingsUI settingsUi;

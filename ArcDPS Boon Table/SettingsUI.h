#pragma once

#include "BuffIds.h"
#include "Settings.h"
#include "extension/BigTable.h"

class SettingsUI {
public:
	void Draw(ImGuiEx::BigTable::ImGuiTable* table, int tableIndex, ImGuiWindow* currentRootWindow);

private:
	float self_color[4]{};
	float _100color[4]{};
	float _0color[4]{};
	bool init = false;
	
	void initialize(int tableIndex);
	bool tableColumnSubMenu(ImGuiEx::BigTable::ImGuiTable* table, const char* label, BoonType type, int beginId) const;

	int position = 0;
	int cornerPosition = 0;
	int selfPanelCornerPosition = 0;
	int anchorPanelCornerPosition = 0;
};

extern SettingsUI settingsUi;

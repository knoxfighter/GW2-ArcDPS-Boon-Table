#pragma once

#include "BuffIds.h"
#include "extension/BigTable.h"

class SettingsUI {
public:
	void Draw(ImGuiEx::BigTable::ImGuiTable* table, int tableIndex, ImGuiWindow* currentRootWindow);

private:
	float windowPadding[2]{};
	char appearAsInOption[128]{};
	
	void initialize(int tableIndex);
	bool tableColumnSubMenu(ImGuiEx::BigTable::ImGuiTable* table, const char* label, BoonType type, int beginId) const;

	int position = 0;
	int cornerPosition = 0;
	int selfPanelCornerPosition = 0;
	int anchorPanelCornerPosition = 0;
};

extern SettingsUI settingsUi;

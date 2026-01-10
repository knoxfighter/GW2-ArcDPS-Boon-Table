#pragma once

#include "Settings.h"

class SettingsUIGlobal {
	friend SettingsUI;
	
public:
	void Draw();

private:
	char shortcut[MaxTableWindowAmount][4]{};

	int killproofKey = 0;
	bool initialized = false;

	void initialize();
};

extern SettingsUIGlobal settingsUiGlobal;

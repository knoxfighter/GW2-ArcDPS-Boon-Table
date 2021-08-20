#pragma once

#include "Settings.h"

class SettingsUIGlobal {
	friend SettingsUI;
	
public:
	void Draw();

private:
	float self_color[4]{};
	float _100color[4]{};
	float _0color[4]{};
	char shortcut[MaxTableWindowAmount][4]{};

	int killproofKey = 0;

	void initialize();
};

extern SettingsUIGlobal settingsUiGlobal;

#pragma once
class SettingsUI {
public:
	void Draw();

private:
	float self_color[4]{};
	bool init = false;
	
	void initialize();
};

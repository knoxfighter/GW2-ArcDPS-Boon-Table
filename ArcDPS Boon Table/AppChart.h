#pragma once
#include <string>
#include "imgui\imgui.h"
#include "Player.h"
#include "Tracker.h"
#include "BuffIds.h"

class AppChart
{
public:
	AppChart();
	~AppChart();

	void Draw(const char* title, bool* p_open, Tracker* tracker);
};

void buffProgressBar(BoonDef current_buff, float current_boon_uptime);
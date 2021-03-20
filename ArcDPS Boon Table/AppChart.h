#pragma once

#include "imgui/imgui.h"
#include "Tracker.h"
#include "BuffIds.h"

class AppChart
{
public:
	std::atomic_bool needSort;
	
	AppChart() = default;

	void Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags);

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const;
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width);
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Entity& entity) const;

	float getEntityDisplayValue(const Tracker& tracker, const Entity& entity, const BoonDef& boon);
};

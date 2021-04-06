#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "Tracker.h"
#include "BuffIds.h"
#include "Settings.h"

class AppChart
{
	friend class AppChartsContainer;
public:
	std::atomic_bool needSort;
	
	AppChart(int new_index) : index(new_index) {};

	void Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags);

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const;
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width);
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Entity& entity) const;

	float getEntityDisplayValue(const Tracker& tracker, const Entity& entity, const BoonDef& boon);

	void removePlayer(size_t playerIndex);
	void addPlayer(size_t playerIndex);

private:
	ImGuiTable* imGuiTable = nullptr;
	int index = 0;
	std::vector<size_t> playerOrder;
};

class AppChartsContainer {
public:
	void removePlayer(size_t playerIndex);
	void addPlayer(size_t playerIndex);
	void sortNeeded();
	void drawAll(Tracker& tracker, ImGuiWindowFlags flags);
	
private:
	AppChart charts[MaxTableWindowAmount] {
		0,1,2,3,4
	};
};

extern AppChartsContainer charts;

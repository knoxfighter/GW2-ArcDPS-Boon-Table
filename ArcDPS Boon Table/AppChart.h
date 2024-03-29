#pragma once

#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "ITracker.h"
#include "BuffIds.h"
#include "Settings.h"
#include "extension/BigTable.h"

class AppChart
{
	friend class AppChartsContainer;
public:
	std::atomic_bool needSort;
	
	AppChart(int new_index) : index(new_index) {};

	void Draw(bool* p_open, ImGuiWindowFlags flags);
	void DrawRow(Alignment alignment, const std::string& charnameText, const char* subgroupText, std::function<float(const BoonDef&)> uptimeFunc, std::function<float()>
	             above90Func, bool hasEntity = false, const IEntity* const entity = nullptr, bool hasColor = false, const ImVec4& color = ImVec4(0, 0, 0, 0));

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const;
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width);
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const IEntity& entity) const;

	float getEntityDisplayValue(const ITracker& tracker, const IEntity& entity, const BoonDef& boon);

	void removePlayer(size_t playerId);
	void addPlayer(size_t playerId);

private:
	ImGuiEx::BigTable::ImGuiTable* imGuiTable = nullptr;
	int index = 0;
	std::vector<size_t> playerOrder;
	uint8_t lastCalculatedHistory = 0;

	uint8_t rowCount = 0;
	float maxHeight = 0;
	float minHeight = 0;
	float titleBarHeight = 0;
	float innerTableCursorPos = 0;
	uint8_t nthTick = 0;

	void endOfRow();
};

class AppChartsContainer {
public:
	void removePlayer(uintptr_t playerId);
	void addPlayer(uintptr_t playerId);
	void clearPlayers();
	void sortNeeded();
	void drawAll(ImGuiWindowFlags flags);
	
private:
	AppChart charts[MaxTableWindowAmount] {
		0,1,2,3,4
	};
};

extern AppChartsContainer charts;

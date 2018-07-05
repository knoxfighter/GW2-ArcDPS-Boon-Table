#pragma once
#include <string>
#include "imgui\imgui.h"
#include "Player.h"
#include "Tracker.h"
#include "BuffIds.h"

class AppChart
{
protected:
	const uintptr_t INDEX_NONE = -1;
	const uintptr_t INDEX_SORTING_BUTTON = -2;
	const uintptr_t INDEX_TOTAL = -3;
public:
	uintptr_t active_player, last_active_player;
	int8_t active_column, last_active_column;
	ImVec4 active_bar_color = ImVec4(1, 1, 1, 1);
	ImVec4 hidden_bar_color = ImVec4(1, 1, 1, 0.3);
	int sorting_collumn;
	
	AppChart();
	~AppChart();

	void Draw(const char* title, bool* p_open, Tracker* tracker, ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

	void buffProgressBar(BoonDef* current_buff, float current_boon_uptime, uintptr_t current_player);

	void highlightedText(uintptr_t player_id, const char* fmt, ...);
	bool highlightedSmallButton(uintptr_t player_id, const char * fmt);
};

bool bShowSubgroups(Tracker* tracker);
bool bShowTotal(Tracker* tracker);
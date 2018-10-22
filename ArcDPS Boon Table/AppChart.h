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
	const uintptr_t INDEX_HIDE_ALL = -4;

	bool show_players = true;
	bool show_subgroups = true;
	bool show_total = true;
public:
	uintptr_t active_player, last_active_player;
	int8_t active_column, last_active_column;
	ImVec4 active_bar_color = ImVec4(1, 1, 1, 1);
	ImVec4 hidden_bar_color = ImVec4(1, 1, 1, 0.3);
	ImVec4 has_boon_color = ImVec4(0.1, 1, 0.1, 1);
	ImVec4 not_have_boon_color = ImVec4(1, 0.1, 0.1, 1);
	int sorting_collumn;
	
	AppChart();
	~AppChart();

	void Draw(const char* title, bool* p_open, Tracker* tracker, ImGuiWindowFlags flags);

	void drawRtClickMenu(Tracker* tracker);

	void buffProgressBar(BoonDef* current_buff, float current_boon_uptime, Player* current_player, uintptr_t current_id);

	void highlightedText(uintptr_t player_id, const char* fmt, ...);
	bool highlightedSmallButton(uintptr_t player_id, const char * fmt);

	void setShowPlayers(bool new_show);
	void setShowSubgroups(bool new_show);
	void setShowTotal(bool new_show);

	bool bShowPlayers(Tracker* tracker);
	bool bShowSubgroups(Tracker* tracker);
	bool bShowTotal(Tracker* tracker);
};

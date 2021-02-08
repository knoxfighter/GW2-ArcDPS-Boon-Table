#pragma once

#include "imgui\imgui.h"
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
	bool show_boon_as_progress_bar = true;

	int current_column = 0;
public:
	ImVec4 has_boon_color = ImVec4(0.1f, 1, 0.1f, 1);
	ImVec4 not_have_boon_color = ImVec4(1, 0.1f, 0.1f, 1);
	
	AppChart() = default;

	void Draw(const char* title, bool* p_open, const Tracker& tracker, ImGuiWindowFlags flags);

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width) const;

	void setShowPlayers(bool new_show);
	void setShowSubgroups(bool new_show);
	void setShowTotal(bool new_show);
	void setShowBoonAsProgressBar(bool new_show);

	[[nodiscard]] bool bShowPlayers(Tracker* tracker);
	[[nodiscard]] bool bShowSubgroups(const Tracker& tracker) const;
	[[nodiscard]] bool getShowSubgroups() const;
	[[nodiscard]] bool bShowTotal() const;
	[[nodiscard]] bool bShowBoonAsProgressBar() const;
	float getPlayerDisplayValue(const Tracker& tracker, const Player& player, const BoonDef& boon);
};

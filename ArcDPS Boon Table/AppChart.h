#pragma once

#include "extension/arcdps_structs.h"
#include "imgui/imgui.h"
#include "Tracker.h"
#include "BuffIds.h"

class AppChart
{
protected:
	bool show_players = true;
	bool show_npcs = true;
	bool show_subgroups = true;
	bool show_total = true;
	bool show_boon_as_progress_bar = true;
	bool size_to_content = true;
	bool alternating_row_bg = true;
	bool show_label = false;
	ProgressBarColoringMode show_colored = ProgressBarColoringMode::Uncolored;
	Alignment alignment = Alignment::Right;
public:
	std::atomic_bool needSort;
	
	AppChart() = default;

	void Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags);

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const;
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width);
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Entity& entity) const;

	void setShowPlayers(bool new_show);
	void setShowSubgroups(bool new_show);
	void setShowTotal(bool new_show);
	void setShowNPCs(bool new_show);
	void setShowBoonAsProgressBar(bool new_show);
	void setSizeToContent(bool new_size_to_content);
	void setShowColored(ProgressBarColoringMode new_colored);
	void setAlternatingRowBg(bool new_alternating_row_bg);
	void setAlignment(Alignment new_alignment);
	void setShowLabel(bool new_show);

	[[nodiscard]] bool bShowPlayers() const;
	[[nodiscard]] bool bShowSubgroups(const Tracker& tracker) const;
	[[nodiscard]] bool getShowSubgroups() const;
	[[nodiscard]] bool bShowTotal() const;
	[[nodiscard]] bool bShowBoonAsProgressBar() const;
	[[nodiscard]] bool bSizeToContent() const;
	[[nodiscard]] bool bAlternatingRowBg() const;
	[[nodiscard]] bool bShowNPCs() const;
	[[nodiscard]] bool bShowLabel() const;
	[[nodiscard]] ProgressBarColoringMode getShowColored() const;
	[[nodiscard]] Alignment getAlignment() const;
	float getEntityDisplayValue(const Tracker& tracker, const Entity& entity, const BoonDef& boon);
};

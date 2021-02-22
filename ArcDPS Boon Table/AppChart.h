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
	bool size_to_content = true;
	bool alternating_row_bg = true;
	ProgressBarColoringMode show_colored = ProgressBarColoringMode::Uncolored;
	Alignment alignment = Alignment::Right;
	std::string alignment_text = to_string(Alignment::Right);

	int current_column = 0;
public:
	ImVec4 has_boon_color = ImVec4(0.1f, 1, 0.1f, 1);
	ImVec4 not_have_boon_color = ImVec4(1, 0.1f, 0.1f, 1);

	std::atomic_bool needSort;
	
	AppChart() = default;

	void Draw(bool* p_open, Tracker& tracker, ImGuiWindowFlags flags);
	void showColorSelectable(ProgressBarColoringMode select_coloring_mode);
	void alignmentSelectable(Alignment select_alignment);

	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, ImVec4 color) const;
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width);
	void buffProgressBar(const BoonDef& current_buff, float current_boon_uptime, float width, const Player& player) const;
	void AlignedTextColumn(const char* text, ...) const;
	void CustomProgressBar(float fraction, const ImVec2& size_arg, const char* overlay) const;

	void setShowPlayers(bool new_show);
	void setShowSubgroups(bool new_show);
	void setShowTotal(bool new_show);
	void setShowBoonAsProgressBar(bool new_show);
	void setSizeToContent(bool new_size_to_content);
	void setShowColored(ProgressBarColoringMode new_colored);
	void setAlternatingRowBg(bool new_alternating_row_bg);
	void setAlignment(Alignment new_alignment);

	[[nodiscard]] bool bShowPlayers() const;
	[[nodiscard]] bool bShowSubgroups(const Tracker& tracker) const;
	[[nodiscard]] bool getShowSubgroups() const;
	[[nodiscard]] bool bShowTotal() const;
	[[nodiscard]] bool bShowBoonAsProgressBar() const;
	[[nodiscard]] bool bSizeToContent() const;
	[[nodiscard]] bool bAlternatingRowBg() const;
	[[nodiscard]] ProgressBarColoringMode getShowColored() const;
	[[nodiscard]] Alignment getAlignment() const;
	float getPlayerDisplayValue(const Tracker& tracker, const Player& player, const BoonDef& boon);
};

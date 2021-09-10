#pragma once

#include <optional>
#include <array>
#include <string>
#include <cstdint>

#include "Helpers.h"
#include "ITracker.h"
#include "modernIni/modernIni/modernIniMacros.h"

import modernIni;

MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE_NO_EXCEPT(ImVec2, x, y)
MODERN_INI_DEFINE_TYPE_NON_INTRUSIVE_NO_EXCEPT(ImVec4, x, y, z, w)

class SettingsUI;
class SettingsUIGlobal;

enum class SizingPolicy {
	SizeToContent,
	SizeContentToWindow,
	ManualWindowSize,
	FINAL_ENTRY
};
std::string to_string(SizingPolicy sizingPolicy);

#define MaxTableWindowAmount 5

class Settings {
	friend SettingsUI;
	friend SettingsUIGlobal;
	
public:
	Settings();
	~Settings();
	void readFromFile();

	bool& isShowChart(int tableIndex);
	[[nodiscard]] Alignment getAlignment(int tableIndex) const;
	[[nodiscard]] bool isShowSubgroups(const ITracker& tracker, int tableIndex) const;
	[[nodiscard]] bool isShowSelfOnTop(int tableIndex) const;
	[[nodiscard]] bool isShowPlayers(int tableIndex) const;
	[[nodiscard]] bool isShowNpcs(int tableIndex) const;
	[[nodiscard]] bool isShowTotal(int tableIndex) const;
	[[nodiscard]] bool isShowBoonAsProgressBar(int tableIndex) const;
	[[nodiscard]] ProgressBarColoringMode getShowColored(int tableIndex) const;
	[[nodiscard]] bool isAlternatingRowBg(int tableIndex) const;
	[[nodiscard]] bool isShowLabel(int tableIndex) const;
	[[nodiscard]] bool isHideHeader(int tableIndex) const;
	[[nodiscard]] SizingPolicy getSizingPolicy(int tableIndex) const;
	[[nodiscard]] float getBoonColumnWidth(int tableIndex) const;
	[[nodiscard]] bool isShowOnlySubgroup(int tableIndex) const;
	[[nodiscard]] bool isShowBackground(int tableIndex) const;
	[[nodiscard]] Position getPosition(int tableIndex) const;
	[[nodiscard]] CornerPosition getCornerPosition(int tableIndex) const;
	[[nodiscard]] const ImVec2& getCornerVector(int tableIndex) const;
	[[nodiscard]] CornerPosition getAnchorPanelCornerPosition(int tableIndex) const;
	[[nodiscard]] CornerPosition getSelfPanelCornerPosition(int tableIndex) const;
	[[nodiscard]] ImGuiID getFromWindowID(int tableIndex) const;
	[[nodiscard]] int getMaxDisplayed(int tableIndex) const;
	[[nodiscard]] const std::optional<ImVec2>& getWindowPadding(int tableIndex) const;
	[[nodiscard]] uint8_t getCurrentHistory(int tableIndex) const;
	[[nodiscard]] int getMaxPlayerLength(int tableIndex) const;
	[[nodiscard]] const std::string& getAppearAsInOption(int tableIndex) const;
	[[nodiscard]] const std::optional<std::string>& getTitleBar(int tableIndex) const;
	[[nodiscard]] bool isScrollbar(int tableIndex) const;
	[[nodiscard]] bool isTablePaddingX(int tableIndex) const;

	[[nodiscard]] std::array<WPARAM, MaxTableWindowAmount> getShortcuts() const;
	[[nodiscard]] const ImVec4& getSelfColor() const;
	[[nodiscard]] const ImVec4& get100Color() const;
	[[nodiscard]] const ImVec4& get0Color() const;
	[[nodiscard]] int getFightsToKeep() const;

	void setShowChart(int tableIndex, bool status);

	// delete copy/move
	Settings(const Settings& other) = delete;
	Settings(Settings&& other) noexcept = delete;
	Settings& operator=(const Settings& other) = delete;
	Settings& operator=(Settings&& other) noexcept = delete;

private:
	struct Table {
		bool show = false;
		bool show_self_on_top = false;
		bool show_players = true;
		bool show_npcs = true;
		bool show_subgroups = true;
		bool show_total = true;
		bool show_uptime_as_progress_bar = true;
		ProgressBarColoringMode show_colored = ProgressBarColoringMode::Uncolored;
		bool alternating_row_bg = true;
		bool show_label = false;
		Alignment alignment = Alignment::Right;
		bool hide_header = false;
		SizingPolicy sizing_policy = SizingPolicy::SizeToContent;
		float boon_column_width = 80.f;
		bool show_only_subgroup = false;
		bool show_background = true;
		Position position = Position::Manual;
		CornerPosition corner_position = CornerPosition::TopLeft;
		ImVec2 corner_vector;
		CornerPosition anchor_panel_corner_position = CornerPosition::TopLeft;
		CornerPosition self_panel_corner_position = CornerPosition::TopLeft;
		ImGuiID from_window_id;
		int max_displayed = 0;
		std::optional<ImVec2> window_padding;
		int max_player_length = 0;
		WPARAM shortcut = 0;
		std::string appear_as_in_option;
		std::optional<std::string> title_bar;
		// history value 1-based (0 = current)
		uint8_t current_history = 0;
		bool scrollbar = true;
		bool table_padding_x = false;

		MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(Table, show, show_self_on_top, show_players, show_npcs, show_subgroups, show_total, 
			show_uptime_as_progress_bar, show_colored, alternating_row_bg, show_label, alignment, hide_header, sizing_policy, boon_column_width,
			show_only_subgroup, show_background, position, corner_position, corner_vector, anchor_panel_corner_position, self_panel_corner_position,
			from_window_id, max_displayed, window_padding, max_player_length, shortcut, appear_as_in_option, title_bar, scrollbar, table_padding_x)
	};
	
	std::optional<ImVec4> self_color;
	std::optional<ImVec4> _100_color;
	std::optional<ImVec4> _0_color;
	int fights_to_keep = 10;
	
	// Table tables[MaxTableWindowAmount]{};
	std::array<Table, MaxTableWindowAmount> tables;

	void saveToFile();
	void convertFromSimpleIni(modernIni::Ini& ini);

	MODERN_INI_DEFINE_TYPE_INTRUSIVE_NO_EXCEPT(Settings, self_color, _100_color, _0_color, fights_to_keep, tables)
};

extern Settings settings;

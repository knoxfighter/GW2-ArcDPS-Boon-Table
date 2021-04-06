#pragma once

#include "Helpers.h"
#include "Tracker.h"
#include "simpleini/SimpleIni.h"

class SettingsUI;

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
	
public:
	Settings();
	~Settings();
	void readFromFile();

	bool& isShowChart(int tableIndex);
	[[nodiscard]] Alignment getAlignment(int tableIndex) const;
	[[nodiscard]] bool isShowSubgroups(const Tracker& tracker, int tableIndex) const;
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

	[[nodiscard]] WPARAM getTableKey() const;
	[[nodiscard]] const ImVec4& getSelfColor() const;

	void setShowChart(int tableIndex, bool status);

	// delete copy/move
	Settings(const Settings& other) = delete;
	Settings(Settings&& other) noexcept = delete;
	Settings& operator=(const Settings& other) = delete;
	Settings& operator=(Settings&& other) noexcept = delete;

private:
	struct Table {
		bool show_chart = false;
		bool show_players = true;
		bool show_npcs = true;
		bool show_subgroups = true;
		bool show_total = true;
		bool show_boon_as_progress_bar = true;
		ProgressBarColoringMode show_colored = ProgressBarColoringMode::Uncolored;
		bool alternating_row_bg = true;
		bool show_label = false;
		Alignment alignment = Alignment::Right;
		bool hide_header = false;
		SizingPolicy sizingPolicy = SizingPolicy::SizeToContent;
		float boon_column_width = 80.f;
		bool show_only_subgroup = false;
	};
	
	CSimpleIniA table_ini;

	WPARAM table_key;
	std::optional<ImVec4> self_color;
	Table tables[MaxTableWindowAmount]{};

	void readTable(int tableIndex);
	void saveToFile();
	void saveTable(int tableIndex);
};

extern Settings settings;

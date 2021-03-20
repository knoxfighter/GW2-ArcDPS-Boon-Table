#pragma once

#include "Helpers.h"
#include "Tracker.h"
#include "simpleini/SimpleIni.h"

class SettingsUI;

enum class SizingPolicy {
	SizeToContent,
	SizeContentToWindow,
	ManualWindowSize
};
std::string to_string(SizingPolicy sizingPolicy);

class Settings {
	friend SettingsUI;
	
public:
	// Show chart public, so it can be changed from everywhere
	bool show_chart = false;
	
	Settings();
	~Settings();

	[[nodiscard]] Alignment getAlignment() const;
	[[nodiscard]] bool isShowSubgroups(const Tracker& tracker) const;
	[[nodiscard]] WPARAM getTableKey() const;
	[[nodiscard]] bool isShowPlayers() const;
	[[nodiscard]] bool isShowNpcs() const;
	[[nodiscard]] bool isShowTotal() const;
	[[nodiscard]] bool isShowBoonAsProgressBar() const;
	[[nodiscard]] ProgressBarColoringMode getShowColored() const;
	[[nodiscard]] bool isAlternatingRowBg() const;
	[[nodiscard]] bool isShowLabel() const;
	[[nodiscard]] bool isHideHeader() const;
	[[nodiscard]] SizingPolicy getSizingPolicy() const;
	[[nodiscard]] float getBoonColumnWidth() const;

	// delete copy/move
	Settings(const Settings& other) = delete;
	Settings(Settings&& other) noexcept = delete;
	Settings& operator=(const Settings& other) = delete;
	Settings& operator=(Settings&& other) noexcept = delete;

private:
	CSimpleIniA table_ini;

	WPARAM table_key;
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

	void readFromFile();
	void saveToFile();
};

extern Settings settings;

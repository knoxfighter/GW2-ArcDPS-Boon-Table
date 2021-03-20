#include "Settings.h"

#include "Lang.h"

Settings settings;

std::string to_string(SizingPolicy sizingPolicy) {
	switch (sizingPolicy) {
	case SizingPolicy::SizeToContent: return lang.translate(LangKey::SizingPolicySizeToContent);
	case SizingPolicy::SizeContentToWindow: return lang.translate(LangKey::SizingPolicySizeContentToWindow);
	case SizingPolicy::ManualWindowSize: return lang.translate(LangKey::SizingPolicyManualWindowSize);
	default: return "Unknown";
	}
}

Settings::Settings() : table_ini(true) {
	readFromFile();
}

Settings::~Settings() {
	saveToFile();
}

Alignment Settings::getAlignment() const {
	return alignment;
}

bool Settings::isShowSubgroups(const Tracker& tracker) const {
	return show_subgroups
		&& tracker.is_squad
		&& tracker.subgroups.size() > 1;
}

WPARAM Settings::getTableKey() const {
	return table_key;
}

bool Settings::isShowPlayers() const {
	return show_players;
}

bool Settings::isShowNpcs() const {
	return show_npcs;
}

bool Settings::isShowTotal() const {
	return show_total;
}

bool Settings::isShowBoonAsProgressBar() const {
	return show_boon_as_progress_bar;
}

ProgressBarColoringMode Settings::getShowColored() const {
	return show_colored;
}

bool Settings::isAlternatingRowBg() const {
	return alternating_row_bg;
}

bool Settings::isShowLabel() const {
	return show_label;
}

bool Settings::isHideHeader() const {
	return hide_header;
}

SizingPolicy Settings::getSizingPolicy() const {
	return sizingPolicy;
}

float Settings::getBoonColumnWidth() const {
	return boon_column_width;
}

void Settings::readFromFile() {
	SI_Error rc = table_ini.LoadFile("addons\\arcdps\\arcdps_table.ini");

	std::string pszValueString = table_ini.GetValue("table", "show", "0");
	show_chart = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "key", "66");
	table_key = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "show_players", "1");
	show_players = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "show_subgroups", "1");
	show_subgroups = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "show_total", "1");
	show_total = std::stoi(pszValueString);

	show_npcs = table_ini.GetBoolValue("table", "show_npcs", true);

	pszValueString = table_ini.GetValue("table", "show_uptime_as_progress_bar", "1");
	show_boon_as_progress_bar = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "show_colored", "0");
	long show_colored_num = table_ini.GetLongValue("table", "show_colored", static_cast<long>(ProgressBarColoringMode::Uncolored));
	show_colored = static_cast<ProgressBarColoringMode>(show_colored_num);

	alternating_row_bg = table_ini.GetBoolValue("table", "alternating_row_bg", true);

	show_label = table_ini.GetBoolValue("table", "show_label");

	long pszValueLong = table_ini.GetLongValue("table", "alignment", static_cast<long>(Alignment::Right));
	alignment = static_cast<Alignment>(pszValueLong);

	hide_header = table_ini.GetBoolValue("table", "hide_header", false);

	pszValueLong = table_ini.GetLongValue("table", "sizing_policy", static_cast<long>(SizingPolicy::SizeToContent));
	sizingPolicy = static_cast<SizingPolicy>(pszValueLong);

	boon_column_width = table_ini.GetDoubleValue("table", "boon_column_width", 80);
}

void Settings::saveToFile() {
	SI_Error rc = table_ini.SetValue("table", "show", std::to_string(show_chart).c_str());

	rc = table_ini.SetValue("table", "show_players", std::to_string(show_players).c_str());
	rc = table_ini.SetValue("table", "show_subgroups", std::to_string(show_subgroups).c_str());
	rc = table_ini.SetValue("table", "show_total", std::to_string(show_total).c_str());
	rc = table_ini.SetBoolValue("table", "show_npcs", show_npcs);
	rc = table_ini.SetValue("table", "show_uptime_as_progress_bar", std::to_string(show_boon_as_progress_bar).c_str());
	rc = table_ini.SetLongValue("table", "show_colored", static_cast<long>(show_colored));
	rc = table_ini.SetBoolValue("table", "show_label", show_label);
	rc = table_ini.SetBoolValue("table", "alternating_row_bg", alternating_row_bg);
	rc = table_ini.SetLongValue("table", "alignment", static_cast<long>(alignment));
	rc = table_ini.SetBoolValue("table", "hide_header", hide_header);
	rc = table_ini.SetLongValue("table", "sizing_policy", static_cast<long>(sizingPolicy));
	rc = table_ini.SetDoubleValue("table", "boon_column_width", boon_column_width);

	rc = table_ini.SaveFile("addons\\arcdps\\arcdps_table.ini");
}

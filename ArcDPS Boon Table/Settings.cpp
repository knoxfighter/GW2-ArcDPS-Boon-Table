#include "Settings.h"

#include "Lang.h"
#include "Helpers.h"

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
}

Settings::~Settings() {
	saveToFile();
}

bool& Settings::isShowChart(int tableIndex) {
	return tables[tableIndex].show_chart;
}

Alignment Settings::getAlignment(int tableIndex) const {
	return tables[tableIndex].alignment;
}

bool Settings::isShowSubgroups(const Tracker& tracker, int tableIndex) const {
	return tables[tableIndex].show_subgroups
		&& tracker.is_squad
		&& tracker.subgroups.size() > 1;
}

WPARAM Settings::getTableKey() const {
	return table_key;
}

bool Settings::isShowPlayers(int tableIndex) const {
	return tables[tableIndex].show_players;
}

bool Settings::isShowNpcs(int tableIndex) const {
	return tables[tableIndex].show_npcs;
}

bool Settings::isShowTotal(int tableIndex) const {
	return tables[tableIndex].show_total;
}

bool Settings::isShowBoonAsProgressBar(int tableIndex) const {
	return tables[tableIndex].show_boon_as_progress_bar;
}

ProgressBarColoringMode Settings::getShowColored(int tableIndex) const {
	return tables[tableIndex].show_colored;
}

bool Settings::isAlternatingRowBg(int tableIndex) const {
	return tables[tableIndex].alternating_row_bg;
}

bool Settings::isShowLabel(int tableIndex) const {
	return tables[tableIndex].show_label;
}

bool Settings::isHideHeader(int tableIndex) const {
	return tables[tableIndex].hide_header;
}

SizingPolicy Settings::getSizingPolicy(int tableIndex) const {
	return tables[tableIndex].sizingPolicy;
}

float Settings::getBoonColumnWidth(int tableIndex) const {
	return tables[tableIndex].boon_column_width;
}

bool Settings::isShowOnlySubgroup(int tableIndex) const {
	return tables[tableIndex].show_only_subgroup;
}

bool Settings::isShowOnlySelf(int tableIndex) const {
	return tables[tableIndex].show_only_self;
}

const ImVec4& Settings::getSelfColor() const {
	if (self_color) {
		return self_color.value();
	} else {
		ImVec4* colors[5];
		arc_export_e5(colors);
		return colors[0][CCOL_LYELLOW];
	}
}

void Settings::setShowChart(int tableIndex, bool status) {
	tables[tableIndex].show_chart = status;
}

void Settings::readFromFile() {
	table_ini.LoadFile("addons\\arcdps\\arcdps_table.ini");

	std::string pszValueString = table_ini.GetValue("general", "key", "66");
	table_key = std::stoi(pszValueString);

	const char* value = table_ini.GetValue("colors", "self_color", "");
	self_color = ImVec4_color_from_string(value);

	for (int i = 0; i < MaxTableWindowAmount; ++i) {
		readTable(i);
	}
}

void Settings::readTable(int tableIndex) {
	Table& table = tables[tableIndex];
	std::string sectionName = "table";

	if (tableIndex > 0) {
		sectionName.append(std::to_string(tableIndex));
	}

	table.show_chart = table_ini.GetBoolValue(sectionName.c_str(), "show");
	table.show_players = table_ini.GetBoolValue(sectionName.c_str(), "show_players", true);
	table.show_subgroups = table_ini.GetBoolValue(sectionName.c_str(), "show_subgroups", true);
	table.show_total = table_ini.GetBoolValue(sectionName.c_str(), "show_total", true);
	table.show_npcs = table_ini.GetBoolValue(sectionName.c_str(), "show_npcs", true);
	table.show_boon_as_progress_bar = table_ini.GetBoolValue(sectionName.c_str(), "show_uptime_as_progress_bar", true);
	long show_colored_num = table_ini.GetLongValue(sectionName.c_str(), "show_colored", static_cast<long>(ProgressBarColoringMode::Uncolored));
	table.show_colored = static_cast<ProgressBarColoringMode>(show_colored_num);
	table.alternating_row_bg = table_ini.GetBoolValue(sectionName.c_str(), "alternating_row_bg", true);
	table.show_label = table_ini.GetBoolValue(sectionName.c_str(), "show_label");
	long pszValueLong = table_ini.GetLongValue(sectionName.c_str(), "alignment", static_cast<long>(Alignment::Right));
	table.alignment = static_cast<Alignment>(pszValueLong);
	table.hide_header = table_ini.GetBoolValue(sectionName.c_str(), "hide_header", false);
	pszValueLong = table_ini.GetLongValue(sectionName.c_str(), "sizing_policy", static_cast<long>(SizingPolicy::SizeToContent));
	table.sizingPolicy = static_cast<SizingPolicy>(pszValueLong);
	table.boon_column_width = table_ini.GetDoubleValue(sectionName.c_str(), "boon_column_width", 80);
	table.show_only_subgroup = table_ini.GetBoolValue(sectionName.c_str(), "show_only_subgroup", false);
	table.show_only_self = table_ini.GetBoolValue(sectionName.c_str(), "show_only_self", false);
}

void Settings::saveToFile() {
	table_ini.SetValue("general", "key", std::to_string(table_key).c_str());

	if (self_color) {
		table_ini.SetValue("colors", "self_color", to_string(self_color.value()).c_str());
	}

	for (int i = 0; i < MaxTableWindowAmount; ++i) {
		saveTable(i);
	}

	table_ini.SaveFile("addons\\arcdps\\arcdps_table.ini");
}

void Settings::saveTable(int tableIndex) {
	const Table& table = tables[tableIndex];
	std::string sectionName = "table";

	if (tableIndex > 0) {
		sectionName.append(std::to_string(tableIndex));
	}

	table_ini.SetBoolValue(sectionName.c_str(), "show", table.show_chart);
	table_ini.SetBoolValue(sectionName.c_str(), "show_players", table.show_players);
	table_ini.SetBoolValue(sectionName.c_str(), "show_subgroups", table.show_subgroups);
	table_ini.SetBoolValue(sectionName.c_str(), "show_total", table.show_total);
	table_ini.SetBoolValue(sectionName.c_str(), "show_npcs", table.show_npcs);
	table_ini.SetBoolValue(sectionName.c_str(), "show_uptime_as_progress_bar", table.show_boon_as_progress_bar);
	table_ini.SetLongValue(sectionName.c_str(), "show_colored", static_cast<long>(table.show_colored));
	table_ini.SetBoolValue(sectionName.c_str(), "show_label", table.show_label);
	table_ini.SetBoolValue(sectionName.c_str(), "alternating_row_bg", table.alternating_row_bg);
	table_ini.SetLongValue(sectionName.c_str(), "alignment", static_cast<long>(table.alignment));
	table_ini.SetBoolValue(sectionName.c_str(), "hide_header", table.hide_header);
	table_ini.SetLongValue(sectionName.c_str(), "sizing_policy", static_cast<long>(table.sizingPolicy));
	table_ini.SetDoubleValue(sectionName.c_str(), "boon_column_width", table.boon_column_width);
	table_ini.SetBoolValue(sectionName.c_str(), "show_only_subgroup", table.show_only_subgroup);
	table_ini.SetBoolValue(sectionName.c_str(), "show_only_self", table.show_only_self);
}


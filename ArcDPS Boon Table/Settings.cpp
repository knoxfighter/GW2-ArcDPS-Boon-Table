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
	try {
		saveToFile();
	} catch(const std::exception& e) {
		std::string err = "BoonTable: Error saving settings to file";
		err.append(e.what());
		arc_log_file(err.c_str());
	}
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

bool Settings::isShowSelfOnTop(int tableIndex) const {
	return tables[tableIndex].show_self_on_top;
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

bool Settings::isShowBackground(int tableIndex) const {
	return tables[tableIndex].show_background;
}

Position Settings::getPosition(int tableIndex) const {
	return tables[tableIndex].position;
}

CornerPosition Settings::getCornerPosition(int tableIndex) const {
	return tables[tableIndex].cornerPosition;
}

const ImVec2& Settings::getCornerVector(int tableIndex) const {
	return tables[tableIndex].cornerVector;
}

CornerPosition Settings::getAnchorPanelCornerPosition(int tableIndex) const {
	return tables[tableIndex].anchorPanelCornerPosition;
}

CornerPosition Settings::getSelfPanelCornerPosition(int tableIndex) const {
	return tables[tableIndex].selfPanelCornerPosition;
}

ImGuiID Settings::getFromWindowID(int tableIndex) const {
	return tables[tableIndex].fromWindowID;
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

const ImVec4& Settings::get100Color() const {
	if (_100color) {
		return _100color.value();
	}
	else {
		return ImVec4(0, 1, 0, (float)125 / 255);
	}
}
const ImVec4& Settings::get0Color() const {
	if (_0color) {
		return _0color.value();
	}
	else {
		return ImVec4(1,0,0, (float)125 / 255);
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
	
	const char* _100Col = table_ini.GetValue("colors", "100%color", "");
	_100color = ImVec4_color_from_string(_100Col);

	const char* _0Col = table_ini.GetValue("colors", "0%color", "");
	_0color = ImVec4_color_from_string(_0Col);

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
	table.show_self_on_top = table_ini.GetBoolValue(sectionName.c_str(), "show_self_on_top", false);
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
	table.show_background = table_ini.GetBoolValue(sectionName.c_str(), "show_background", true);
	long positionInt = table_ini.GetLongValue(sectionName.c_str(), "position", static_cast<long>(Position::Manual));
	table.position = static_cast<Position>(positionInt);
	long cornerPositionInt = table_ini.GetLongValue(sectionName.c_str(), "corner_position", static_cast<long>(CornerPosition::TopLeft));
	table.cornerPosition = static_cast<CornerPosition>(cornerPositionInt);
	
	
	ImVec2 cornerVector;
	CornerPosition anchorPanelCornerPosition = CornerPosition::TopLeft;
	CornerPosition selfPanelCornerPosition = CornerPosition::TopLeft;
	ImGuiID fromWindowID;
}

void Settings::saveToFile() {
	table_ini.SetValue("general", "key", std::to_string(table_key).c_str());

	if (self_color) {
		table_ini.SetValue("colors", "self_color", to_string(self_color.value()).c_str());
	}

	if (_100color) {
		table_ini.SetValue("colors", "100%color", to_string(_100color.value()).c_str());
	}

	if (_0color) {
		table_ini.SetValue("colors", "0%color", to_string(_0color.value()).c_str());
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
	table_ini.SetBoolValue(sectionName.c_str(), "show_self_on_top", table.show_self_on_top);
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
	table_ini.SetBoolValue(sectionName.c_str(), "show_background", table.show_background);
}


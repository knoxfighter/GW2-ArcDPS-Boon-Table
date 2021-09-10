#include "Settings.h"

#include <fstream>

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

Settings::Settings() {
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
	return tables[tableIndex].show;
}

Alignment Settings::getAlignment(int tableIndex) const {
	return tables[tableIndex].alignment;
}

bool Settings::isShowSubgroups(const ITracker& tracker, int tableIndex) const {
	return tables[tableIndex].show_subgroups
		&& tracker.isSquad()
		&& tracker.getSubgroups().size() > 1;
}

bool Settings::isShowSelfOnTop(int tableIndex) const {
	return tables[tableIndex].show_self_on_top;
}

std::array<WPARAM, MaxTableWindowAmount> Settings::getShortcuts() const {
	std::array<WPARAM, MaxTableWindowAmount> ret;
	for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
		ret[i] = tables[i].shortcut;
	}
	return ret;
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
	return tables[tableIndex].show_uptime_as_progress_bar;
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
	return tables[tableIndex].sizing_policy;
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
	return tables[tableIndex].corner_position;
}

const ImVec2& Settings::getCornerVector(int tableIndex) const {
	return tables[tableIndex].corner_vector;
}

CornerPosition Settings::getAnchorPanelCornerPosition(int tableIndex) const {
	return tables[tableIndex].anchor_panel_corner_position;
}

CornerPosition Settings::getSelfPanelCornerPosition(int tableIndex) const {
	return tables[tableIndex].self_panel_corner_position;
}

ImGuiID Settings::getFromWindowID(int tableIndex) const {
	return tables[tableIndex].from_window_id;
}

int Settings::getMaxDisplayed(int tableIndex) const {
	return tables[tableIndex].max_displayed;
}

const std::optional<ImVec2>& Settings::getWindowPadding(int tableIndex) const {
	return tables[tableIndex].window_padding;
}

uint8_t Settings::getCurrentHistory(int tableIndex) const {
	return tables[tableIndex].current_history;
}

int Settings::getMaxPlayerLength(int tableIndex) const {
	return tables[tableIndex].max_player_length;
}

const std::string& Settings::getAppearAsInOption(int tableIndex) const {
	return tables[tableIndex].appear_as_in_option;
}

const std::optional<std::string>& Settings::getTitleBar(int tableIndex) const {
	return tables[tableIndex].title_bar;
}

bool Settings::isScrollbar(int tableIndex) const {
	return tables[tableIndex].scrollbar;
}

bool Settings::isTablePaddingX(int tableIndex) const {
	return tables[tableIndex].table_padding_x;
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
	if (_100_color) {
		return _100_color.value();
	}
	else {
		return ImVec4(0, 1, 0, (float)125 / 255);
	}
}
const ImVec4& Settings::get0Color() const {
	if (_0_color) {
		return _0_color.value();
	}
	else {
		return ImVec4(1,0,0, (float)125 / 255);
	}
}

int Settings::getFightsToKeep() const {
	return fights_to_keep;
}

void Settings::setShowChart(int tableIndex, bool status) {
	tables[tableIndex].show = status;
}

void Settings::readFromFile() {
	std::ifstream stream("addons\\arcdps\\arcdps_table.ini");

	if (!stream.is_open()) {
		// Do not load if file was not opened/found

		// load defaults
		tables[0].shortcut = 66;
		
		return;
	}
	
	modernIni::Ini ini;

	stream >> ini;

	ini.get_to(*this);

	convertFromSimpleIni(ini);

	// convertion/defaults for shortcuts
	if (ini.has("table_key")) {
		ini.at("table_key").get_to(tables[0].shortcut);
	} else {
		if (!ini.has("tables")) {
			tables[0].shortcut = 66;
		} else {
			auto& iniTables = ini.at("tables");
			if (!iniTables.has("0")) {
				tables[0].shortcut = 66;
			} else {
				auto& zero = iniTables.at("0");
				if (!zero.has("shortcut")) {
					tables[0].shortcut = 66;
				}
			}
		}
	}

	// fix for 0-value
	if (self_color) {
		if (self_color->x == 0 && self_color->y == 0 && self_color->z == 0 && self_color->w == 0) {
			self_color.reset();
		}
	}
}

void Settings::convertFromSimpleIni(modernIni::Ini& ini) {
	if (ini.has("general")) {
		auto& general = ini.at("general");
		if (general.has("key")) {
			general.at("key").get_to(tables[0].shortcut);
		}
	}

	if (ini.has("colors")) {
		auto& colors = ini.at("colors");
		if (colors.has("self_color")) {
			const std::string self_color_str = colors.at("self_color").get<std::string>();
			self_color = ImVec4_color_from_string(self_color_str);
		}
		if (colors.has("100%color")) {
			const std::string _100_color_str = colors.at("100%color").get<std::string>();
			_100_color = ImVec4_color_from_string(_100_color_str);
		}
		if (colors.has("0%color")) {
			const std::string _0_color_str = colors.at("0%color").get<std::string>();
			_0_color = ImVec4_color_from_string(_0_color_str);
		}
	}

	if (ini.has("table")) {
		auto& table = ini.at("table");
		Table& t = tables[0];
		table.get_to(t);
	}

	if (ini.has("table1")) {
		auto& table = ini.at("table1");
		Table& t = tables[1];
		table.get_to(t);
	}

	if (ini.has("table2")) {
		auto& table = ini.at("table2");
		Table& t = tables[2];
		table.get_to(t);
	}

	if (ini.has("table3")) {
		auto& table = ini.at("table3");
		Table& t = tables[3];
		table.get_to(t);
	}

	if (ini.has("table4")) {
		auto& table = ini.at("table4");
		Table& t = tables[4];
		table.get_to(t);
	}
}

void Settings::saveToFile() {
	std::ofstream stream("addons\\arcdps\\arcdps_table.ini");

	if (!stream.is_open()) {
		// Do not load if file was not opened/found
		return;
	}

	modernIni::Ini ini(*this);

	stream << ini;

	stream.flush();
	stream.close();
}

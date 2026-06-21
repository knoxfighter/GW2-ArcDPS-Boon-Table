#include "Settings.h"

#include <ArcdpsUnofficialExtras/Definitions.h>
#include <fstream>

#include "Helpers.h"
#include "Lang.h"

Settings settings;

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

void Settings::setCurrentHistory(int tableIndex, uint8_t currentHistory) {
	tables[tableIndex].current_history = currentHistory;
}

void Settings::setDefaultWindowPadding(int tableIndex, ImVec2 defaultWindowPadding) {
	tables[tableIndex].window_padding_default = std::move(defaultWindowPadding);
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

ImVec4 Settings::getSelfColor() const {
	if (self_color) {
		return self_color.value();
	} else {
		ImVec4* colors[5];
		arc_export_e5(colors);
		return colors[0][CCOL_LYELLOW];
	}
}

ImVec4 Settings::get100Color() const {
	if (_100_color) {
		return _100_color.value();
	} else {
		return ImVec4(0, 1, 0, (float) 125 / 255);
	}
}
ImVec4 Settings::get0Color() const {
	if (_0_color) {
		return _0_color.value();
	} else {
		return ImVec4(1, 0, 0, (float) 125 / 255);
	}
}

int Settings::getFightsToKeep() const {
	return fights_to_keep;
}

const std::string& Settings::getLanguage() const {
	return language2;
}

const std::string& Settings::getGameLanguage() const {
	return gameLanguage;
}

void Settings::setLanguage(std::string newLanguage) {
	language2 = std::move(newLanguage);

	ArcdpsExtension::Localization::instance().ChangeLanguage(language2 == Lang::LikeGame ? getGameLanguage() : language2);
}

void Settings::setGameLanguage(std::string newLanguage) {
	gameLanguage = std::move(newLanguage);

	if (settings.getLanguage() == Lang::LikeGame) {
		ArcdpsExtension::Localization::instance().ChangeLanguage(gameLanguage);
	}
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

	std::ignore = ini.get_to(*this);

	// fix for 0-value
	if (self_color) {
		if (self_color->x == 0 && self_color->y == 0 && self_color->z == 0 && self_color->w == 0) {
			self_color.reset();
		}
	}

	// migrate to new Language Codes
	if (ini.contains("language")) {
		auto lang = ini.at("language");
		if (lang) {
			auto set = lang.value().get().get<uint32_t>();
			if (set) {
				switch (set.value()) {
					case 1:
						language2 = Lang::LikeGame;
						break;
					default:
						language2 = ArcdpsExtension::Localization::ToLangCode(static_cast<Language>(set.value()));
				}
			}
		}
	}

	// At the end, check if the current translation is even allowed
	if (language2 != Lang::LikeGame) {
		if (!ArcdpsExtension::Localization::instance().GetLanguages().contains(language2)) {
			ARC_LOG(std::format("language '{}' not available, resetting to Default", language2).c_str());
			ARC_LOG_FILE(std::format("language '{}' not available, resetting to Default", language2).c_str());
			language2 = Lang::LikeGame;
		}
	}
	// and activate the saved language
	setLanguage(language2);
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

#include "Lang.h"

#include <fstream>

import modernIni;

Lang lang;

std::string Lang::translate(LangKey key) {
	return langMap.at(key);
}

void Lang::readFromFile() {
	std::ifstream stream("addons\\arcdps\\arcdps_table_lang.ini");

	if (!stream.is_open()) {
		// file not opened/exists
		return;
	}

	modernIni::Ini ini;

	stream >> ini;

	if (ini.has("translation")) {
		ini.at("translation").get_to(langMap);
	}
}

void Lang::saveToFile() {
#ifdef _DEBUG
	std::ofstream stream("addons\\arcdps\\arcdps_table_lang.ini");

	modernIni::Ini ini;

	ini["translation"] = langMap;

	stream << ini;
	
	stream.flush();
	stream.close();
#endif
}

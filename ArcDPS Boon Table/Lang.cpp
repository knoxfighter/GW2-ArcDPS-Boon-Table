#include "Lang.h"

#include "simpleini/SimpleIni.h"

Lang lang;

std::string Lang::translate(LangKey key) {
	return translations.at(key);
}

Lang::Lang() : lang_ini(true) {
	readFromFile();
}

Lang::~Lang() {
#ifdef _DEBUG
	saveToFile();
#endif
}

void Lang::readFromFile() {
	SI_Error rc = lang_ini.LoadFile("addons\\arcdps\\arcdps_table_lang.ini");
	// copy defaults if file does not exist or is unreadable
	if (rc < 0) {
		translations = langDefaults;
		return;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(LangKey::FINAL_ENTRY); ++i) {
		LangKey key = static_cast<LangKey>(i);
		std::string trans = lang_ini.GetValue("translation", langIniNames.at(key).c_str(), langDefaults.at(key).c_str());
		translations.try_emplace(key, trans);
	}
}

void Lang::saveToFile() {
	for (const auto& translation : translations) {
		lang_ini.SetValue("translation", langIniNames.at(translation.first).c_str(), translation.second.c_str());
	}

	lang_ini.SaveFile("addons\\arcdps\\arcdps_table_lang.ini");
}

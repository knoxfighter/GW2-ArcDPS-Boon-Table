#pragma once

#include <map>
#include <string>

#include "extension/map.h"
#include "simpleini/SimpleIni.h"

#define BUILD_MAP(val1, val2, key) {key::val1, #val1}
#define LITERAL(val1, val2) val1,
#define BUILD_DEFAULTS(val1, val2, key) {key::val1, val2}

#define BUILD_ENUM_INI_NAMES(key, ...) \
	enum class key { \
		MAP2(LITERAL, __VA_ARGS__) \
	}; \
	static const std::map<key, std::string> langIniNames = { \
		MAP_LIST_UD2( \
			BUILD_MAP, \
			key, \
			__VA_ARGS__ \
		) \
	}; \
	static const std::map<key, std::string> langDefaults = { \
		MAP_LIST_UD2( \
			BUILD_DEFAULTS, \
			key, \
			__VA_ARGS__ \
		) \
	};

BUILD_ENUM_INI_NAMES(
	LangKey,
	WindowHeader, "Boon Table",
	Left, "Left",
	Right, "Right",
	Center, "Center",
	Unaligned, "Unaligned",
	ShowChart, "Boon Table",
	SettingsPlayers, "Players",
	SettingsSubgroups, "Subgroups",
	SettingsTotal, "Total",
	SettingsNPC, "NPCs",
	SettingsShowProgressBar, "Show values as progress bar",
	SettingsAlwaysResize, "Always resize window to content",
	SettingsAlternatingRow, "Alternating Row Background",
	SettingsColoringMode, "Coloring Mode",
	ColoringModeDefault, "Default",
	ColoringModeByProfession, "By Profession",
	ColoringModeByPercentage, "By Percentage",
	SettingsAlignment, "Alignment",
	SettingsShowLabel, "Show header with text instead of images",
	SettingsHideHeader, "Hide window header",
	SettingsSizingPolicy, "Sizing Policy",
	SettingsBoonColumnWidth, "Boon column width",
	SettingsShowOnlySubgroup, "Show only your Subgroup",
	SettingsShowOnlySelf, "Show only yourself",
	SettingsSelfColor, "Self Color",
	SettingsWidthSlideTooltip, "Press CTRL+Leftclick to manually input the number.",
	SettingsColumnSetup, "Column Setup",
	SettingsStyle, "Style",

	SizingPolicySizeToContent, "Autoresize window to content",
	SizingPolicySizeContentToWindow, "Size content to window",
	SizingPolicyManualWindowSize, "Manual window sizing",

	NameColumnHeader, "Name",
	SubgroupColumnHeader, "Sub",
	SubgroupNameColumnValue, "Subgroup",
	TotalNameColumnValue, "TOTAL",
	TotalSubgroupColumnValue, "ALL",
	NPCSubgroupColumnValue, "NPC",

	BoonTypeBoon, "Boons",
	BoonTypeTrait, "Traits",
	BoonTypeBanner, "Banner",
	BoonTypeSprit, "Spirits",
	BoonTypeSkill, "Skills",
	BoonTypeSignet, "Signets",
	BoonTypeOther, "Other",

	BuffMight, "Might",
	BuffFury, "Fury",
	BuffRegeneration, "Regeneration",
	BuffProtection, "Protection",
	BuffQuickness, "Quickness",
	BuffAlacrity, "Alacrity",
	BuffRetaliation, "Retaliation",
	BuffVigor, "Vigor",
	BuffStability, "Stability",
	BuffAegis, "Aegis",
	BuffSwiftness, "Swiftness",
	BuffResistance, "Resistance",
	BuffEmpowerAllies, "Empower Allies",
	BuffPinpointDistribution, "Pinpoint Distribution",
	BuffAssassinsPresence, "Assassin's Presence",
	BuffSpotter, "Spotter",
	BuffSoothingMist, "Soothing Mist",
	BuffVampiricAura, "Vampiric Aura",
	BuffStrengthInNumbers, "Strength in Numbers",
	BuffStrength, "Banner of Strength",
	BuffBannerDiscipline, "Banner of Discipline",
	BuffBannerTactics, "Banner of Tactics",
	BuffBannerDefense, "Banner Defense",
	BuffSpiritFrost, "Frostspirit",
	BuffSpiritSun, "Sunspirit",
	BuffSpiritStone, "Stonespirit",
	BuffSpiritStorm, "Stormspirit",
	BuffSpiritWater, "Waterspirit",
	BuffSignetInspiration, "Signet of Inspiration",
	BuffStealth, "Stealth",
	BuffSuperspeed, "Superspeed",
	Above90Hp, "above 90% HP",

	UpdateWindowHeader, "Arcdps Boon Table Plugin Update",
	UpdateDesc, "A new update for the Boon Table plugin is available.",
	UpdateCurrentVersion, "Current version",
	UpdateNewVersion, "New version",

	// always last element
	// ALWAYS UPDATE AFTER CHANGING THE ENUM
	FINAL_ENTRY, ""
)

class Lang {
public:
	std::string translate(LangKey key);

	Lang();
	~Lang();

	// copy/move delete
	Lang(const Lang& other) = delete;
	Lang(Lang&& other) noexcept = delete;
	Lang& operator=(const Lang& other) = delete;
	Lang& operator=(Lang&& other) noexcept = delete;

private:
	std::map<LangKey, std::string> translations;
	CSimpleIniA lang_ini;

	void readFromFile();
	void saveToFile();
};

extern Lang lang;

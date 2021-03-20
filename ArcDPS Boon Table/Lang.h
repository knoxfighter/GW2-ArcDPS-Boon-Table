#pragma once

#include <map>
#include <string>

#include "extension/map.h"
#include "simpleini/SimpleIni.h"

#define BUILD_MAP(val, key) {key::val, #val}
#define LITERAL(val) val,
#define BUILD_ENUM_INI_NAMES(key, ...) \
	enum class key { \
		MAP(LITERAL, __VA_ARGS__) \
	}; \
	static const std::map<key, std::string> langIniNames = { \
		MAP_LIST_UD( \
			BUILD_MAP, \
			key, \
			__VA_ARGS__ \
		) \
	};

BUILD_ENUM_INI_NAMES(
	LangKey,
	WindowHeader,
	Left,
	Right,
	Center,
	Unaligned,
	ShowChart,
	SettingsPlayers,
	SettingsSubgroups,
	SettingsTotal,
	SettingsNPC,
	SettingsShowProgressBar,
	SettingsAlwaysResize,
	SettingsAlternatingRow,
	SettingsColoringMode,
	ColoringModeDefault,
	ColoringModeByProfession,
	ColoringModeByPercentage,
	SettingsAlignment,
	SettingsShowLabel,
	SettingsHideHeader,
	SettingsSizingPolicy,
	SettingsBoonColumnWidth,

	SizingPolicySizeToContent,
	SizingPolicySizeContentToWindow,
	SizingPolicyManualWindowSize,

	NameColumnHeader,
	SubgroupColumnHeader,
	SubgroupNameColumnValue,
	TotalNameColumnValue,
	TotalSubgroupColumnValue,
	NPCSubgroupColumnValue,

	BuffMight,
	BuffFury,
	BuffRegeneration,
	BuffProtection,
	BuffQuickness,
	BuffAlacrity,
	BuffRetaliation,
	BuffVigor,
	BuffStability,
	BuffAegis,
	BuffSwiftness,
	BuffResistance,
	BuffEmpowerAllies,
	BuffPinpointDistribution,
	BuffAssassinsPresence,
	BuffSpotter,
	BuffSoothingMist,
	BuffVampiricAura,
	BuffStrengthInNumbers,
	BuffStrength,
	BuffBannerDiscipline,
	BuffBannerTactics,
	BuffBannerDefense,
	BuffSpiritFrost,
	BuffSpiritSun,
	BuffSpiritStone,
	BuffSpiritStorm,
	BuffSpiritWater,
	BuffSignetInspiration,
	BuffStealth,
	BuffSuperspeed,

	// always last element
	// ALWAYS UPDATE AFTER CHANGING THE ENUM
	FINAL_ENTRY
)

static const std::map<LangKey, std::string> langDefaults = {
		{LangKey::WindowHeader, "Boon Table"},
		{LangKey::Left, "Left"},
		{LangKey::Center, "Center"},
		{LangKey::Right, "Right"},
		{LangKey::Unaligned, "Unaligned"},
		{LangKey::ShowChart, "Boon Table"},
		{LangKey::SettingsPlayers, "Players"},
		{LangKey::SettingsSubgroups, "Subgroups"},
		{LangKey::SettingsTotal, "Total"},
		{LangKey::SettingsNPC, "NPCs"},
		{LangKey::SettingsShowProgressBar, "Show values as progress bar"},
		{LangKey::SettingsAlwaysResize, "Always resize window to content"},
		{LangKey::SettingsAlternatingRow, "Alternating Row Background"},
		{LangKey::SettingsColoringMode, "Coloring Mode"},
		{LangKey::ColoringModeDefault, "Default"},
		{LangKey::ColoringModeByProfession, "By Profession"},
		{LangKey::ColoringModeByPercentage, "By Percentage"},
		{LangKey::SettingsAlignment, "Alignment"},
		{LangKey::SettingsShowLabel, "Show header with text instead of images"},
		{LangKey::SettingsHideHeader, "Hide window header"},
		{LangKey::SettingsSizingPolicy, "Sizing Policy"},

		{LangKey::SizingPolicySizeToContent, "Autoresize window to content"},
		{LangKey::SizingPolicySizeContentToWindow, "Size content to window"},
		{LangKey::SizingPolicyManualWindowSize, "Manual window sizing"},
		{LangKey::SettingsBoonColumnWidth, "Boon column width"},

		{LangKey::NameColumnHeader, "Name"},
		{LangKey::SubgroupColumnHeader, "Sub"},
		{LangKey::SubgroupNameColumnValue, "Subgroup"},
		{LangKey::TotalNameColumnValue, "TOTAL"},
		{LangKey::TotalSubgroupColumnValue, "ALL"},
		{LangKey::NPCSubgroupColumnValue, "NPC"},
	
		{LangKey::BuffMight, "Might"},
		{LangKey::BuffFury, "Fury"},
		{LangKey::BuffRegeneration, "Regen"},
		{LangKey::BuffProtection, "Prot"},
		{LangKey::BuffQuickness, "Quick"},
		{LangKey::BuffAlacrity, "Alac"},
		{LangKey::BuffRetaliation, "Retal"},
		{LangKey::BuffVigor, "Vigor"},
		{LangKey::BuffStability, "Stability"},
		{LangKey::BuffAegis, "Aegis"},
		{LangKey::BuffSwiftness, "Swiftness"},
		{LangKey::BuffResistance, "Resistance"},
		{LangKey::BuffEmpowerAllies, "EA"},
		{LangKey::BuffPinpointDistribution, "PP"},
		{LangKey::BuffAssassinsPresence, "AP"},
		{LangKey::BuffSpotter, "Spotter"},
		{LangKey::BuffSoothingMist, "Sooth Mist"},
		{LangKey::BuffVampiricAura, "Vam Aura"},
		{LangKey::BuffStrengthInNumbers, "Stren Num"},
		{LangKey::BuffStrength, "Strength"},
		{LangKey::BuffBannerDiscipline, "Discipline"},
		{LangKey::BuffBannerTactics, "Tactics"},
		{LangKey::BuffBannerDefense, "Defense"},
		{LangKey::BuffSpiritFrost, "Frost"},
		{LangKey::BuffSpiritSun, "Sun"},
		{LangKey::BuffSpiritStone, "Stone"},
		{LangKey::BuffSpiritStorm, "Storm"},
		{LangKey::BuffSpiritWater, "Water"},
		{LangKey::BuffSignetInspiration, "Inspiration"},
		{LangKey::BuffStealth, "Stealth"},
		{LangKey::BuffSuperspeed, "Superspeed"},
};

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

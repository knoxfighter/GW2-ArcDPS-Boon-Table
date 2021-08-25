#pragma once

#include <map>
#include <string>

#include "modernIni/modernIni/modernIniMacros.h"

#define LITERAL(val) val

#define LANG_KEY_ENUM(key, ...) \
	enum class key { \
		MAP_LIST(LITERAL, __VA_ARGS__) \
	}; \
	MODERN_INI_SERIALIZE_ENUM(key, __VA_ARGS__)

LANG_KEY_ENUM(
	LangKey,
	WindowHeader,
	Left,
	Right,
	Center,
	Unaligned,
	ShowChart,

	SettingsWindowName,
	SettingsSelfOnTop,
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
	SettingsShowOnlySubgroup,
	SettingsSelfColor,
	SettingsWidthSlideTooltip,
	SettingsColumnSetup,
	SettingsStyle,
	Settings100Color,
	Settings0Color,
	SettingsShowBackground,
	SettingsResetTableColumns,
	SizingPolicySizeToContent,
	SizingPolicySizeContentToWindow,
	SizingPolicyManualWindowSize,
	SettingsWindowPadding,

	SettingsPosition,
	SettingsFromAnchorPanelCorner,
	SettingsToThisPanelCorner,
	SettingsFromWindowName,
	SettingsDisplay,
	SettingsMaxDisplayed,
	SettingsMaxDisplayedPopup,
	SettingsMaxPlayerLength,

	SettingsHistory,
	SettingsHistoryCurrent,

	SettingsAppearAsInOption,
	SettingsTitleBar,

	// Make sure, this stays in order!
	SettingsShortcut,
	SettingsShortcut1,
	SettingsShortcut2,
	SettingsShortcut3,
	SettingsShortcut4,

	NameColumnHeader,
	SubgroupColumnHeader,
	SubgroupNameColumnValue,
	TotalNameColumnValue,
	TotalSubgroupColumnValue,
	NPCSubgroupColumnValue,

	BoonTypeBoon,
	BoonTypeWarrior,
	BoonTypeRevenant,
	BoonTypeGuardian,
	BoonTypeEngineer,
	BoonTypeRanger,
	BoonTypeElementalist,
	BoonTypeMesmer,
	BoonTypeNecromancer,
	BoonTypeAura,
	BoonTypeOther,

	BuffMight,
	BuffFury,
	BuffRegeneration,
	BuffProtection,
	BuffQuickness,
	BuffAlacrity,
	BuffResolution,
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
	Above90Hp,
	BuffSignetResolve,
	BuffSignetBane,
	BuffSignetJudgment,
	BuffSignetMercy,
	BuffSignetWrath,
	BuffSignetCourage,
	BuffDolyakStance,
	BuffGriffonStance,
	BuffMoaStance,
	BuffVultureStance,
	BuffBearStance,
	BuffOneWolfPack,
	BuffSoulcleavesSummit,
	BuffRazorclawsRage,
	BuffBreakrazorsBastion,
	BuffChaosAura,
	BuffDarkAura,
	BuffFireAura,
	BuffFrostAura,
	BuffLightAura,
	BuffMagneticAura,
	BuffShockingAura,
	BuffSoulBarbs,
	BuffSpectralArmor,
	BuffInspiringVirtue,
	BuffEternalOasis,
	BuffUnbrokenLines,
	BuffRiteOfTheGreatDwarf,
	BuffBulwarkGyro,
	BuffSymbolicAvenger,
	BuffInvigoratedBulwark,
	BuffStickingTogether,
	BuffRigorousCertainty,

	PositionManual,
	PositionScreenRelative,
	PositionWindowRelative,

	CornerPositionTopLeft,
	CornerPositionTopRight,
	CornerPositionBottomLeft,
	CornerPositionBottomRight,

	UpdateWindowHeader,
	UpdateDesc,
	UpdateCurrentVersion,
	UpdateNewVersion,
	UpdateOpenPage,
	UpdateAutoButton,
	UpdateInProgress,
	UpdateRestartPending,
	UpdateError,

	Unknown,

	// always last element
	FINAL_ENTRY
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
	void readFromFile();
	void saveToFile();

	std::map<LangKey, std::string> langMap{
		{LangKey::WindowHeader, "Boon Table"},
		{LangKey::Left, "Left"},
		{LangKey::Right, "Right"},
		{LangKey::Center, "Center"},
		{LangKey::Unaligned, "Unaligned"},
		{LangKey::ShowChart, "Boon Table"},
		{LangKey::SettingsWindowName, "Boon Table Settings"},
		{LangKey::SettingsSelfOnTop, "Show self on top"},
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
		{LangKey::SettingsBoonColumnWidth, "Boon column width"},
		{LangKey::SettingsShowOnlySubgroup, "Show only your Subgroup"},
		{LangKey::SettingsSelfColor, "Self Color"},
		{LangKey::SettingsWidthSlideTooltip, "Press CTRL+Leftclick to manually input the number."},
		{LangKey::SettingsColumnSetup, "Column Setup"},
		{LangKey::SettingsStyle, "Style"},
		{LangKey::Settings100Color, "100% color"},
		{LangKey::Settings0Color, "0% color"},
		{LangKey::SettingsShowBackground, "Background"},
		{LangKey::SettingsResetTableColumns, "Reset table columns"},
		{LangKey::SettingsHistoryCurrent, "current"},
		{LangKey::SettingsHistory, "history"},
		{LangKey::SettingsWindowPadding, "padding"},
		{LangKey::SizingPolicySizeToContent, "Autoresize window to content"},
		{LangKey::SizingPolicySizeContentToWindow, "Size content to window"},
		{LangKey::SizingPolicyManualWindowSize, "Manual window sizing"},
		{LangKey::SettingsMaxPlayerLength, "Max player length"},
		{LangKey::SettingsAppearAsInOption, "appear as in option"},
		{LangKey::SettingsTitleBar, "title bar"},
		{LangKey::SettingsShortcut, "Shortcut #0"},
		{LangKey::SettingsShortcut1, "Shortcut #1"},
		{LangKey::SettingsShortcut2, "Shortcut #2"},
		{LangKey::SettingsShortcut3, "Shortcut #3"},
		{LangKey::SettingsShortcut4, "Shortcut #4"},
		{LangKey::NameColumnHeader, "Name"},
		{LangKey::SubgroupColumnHeader, "Sub"},
		{LangKey::SubgroupNameColumnValue, "Subgroup"},
		{LangKey::TotalNameColumnValue, "Total"},
		{LangKey::TotalSubgroupColumnValue, "All"},
		{LangKey::NPCSubgroupColumnValue, "NPC"},
		{LangKey::BoonTypeBoon, "Boons"},
		{LangKey::BoonTypeWarrior, "Warrior"},
		{LangKey::BoonTypeRevenant, "Revenant"},
		{LangKey::BoonTypeGuardian, "Guardian"},
		{LangKey::BoonTypeEngineer, "Engineer"},
		{LangKey::BoonTypeRanger, "Ranger"},
		{LangKey::BoonTypeElementalist, "Elementalist"},
		{LangKey::BoonTypeMesmer, "Mesmer"},
		{LangKey::BoonTypeNecromancer, "Necromancer"},
		{LangKey::BoonTypeAura, "Auras"},
		{LangKey::BoonTypeOther, "Other"},
		{LangKey::BuffMight, "Might"},
		{LangKey::BuffFury, "Fury"},
		{LangKey::BuffRegeneration, "Regeneration"},
		{LangKey::BuffProtection, "Protection"},
		{LangKey::BuffQuickness, "Quickness"},
		{LangKey::BuffAlacrity, "Alacrity"},
		{LangKey::BuffResolution, "Resolution"},
		{LangKey::BuffVigor, "Vigor"},
		{LangKey::BuffStability, "Stability"},
		{LangKey::BuffAegis, "Aegis"},
		{LangKey::BuffSwiftness, "Swiftness"},
		{LangKey::BuffResistance, "Resistance"},
		{LangKey::BuffEmpowerAllies, "Empower Allies"},
		{LangKey::BuffPinpointDistribution, "Pinpoint Distribution"},
		{LangKey::BuffAssassinsPresence, "Assassin's Presence"},
		{LangKey::BuffSpotter, "Spotter"},
		{LangKey::BuffSoothingMist, "Soothing Mist"},
		{LangKey::BuffVampiricAura, "Vampiric Aura"},
		{LangKey::BuffStrengthInNumbers, "Strength in Numbers"},
		{LangKey::BuffStrength, "Banner of Strength"},
		{LangKey::BuffBannerDiscipline, "Banner of Discipline"},
		{LangKey::BuffBannerTactics, "Banner of Tactics"},
		{LangKey::BuffBannerDefense, "Banner Defense"},
		{LangKey::BuffSpiritFrost, "Frostspirit"},
		{LangKey::BuffSpiritSun, "Sunspirit"},
		{LangKey::BuffSpiritStone, "Stonespirit"},
		{LangKey::BuffSpiritStorm, "Stormspirit"},
		{LangKey::BuffSpiritWater, "Waterspirit"},
		{LangKey::BuffSignetInspiration, "Signet of Inspiration"},
		{LangKey::BuffStealth, "Stealth"},
		{LangKey::BuffSuperspeed, "Superspeed"},
		{LangKey::Above90Hp, "above 90% HP"},
		{LangKey::BuffSignetResolve, "Signet of Resolve"},
		{LangKey::BuffSignetBane, "Bane Signet"},
		{LangKey::BuffSignetJudgment, "Signet of Judgment"},
		{LangKey::BuffSignetMercy, "Signet of Mercy"},
		{LangKey::BuffSignetWrath, "Signet of Wrath"},
		{LangKey::BuffSignetCourage, "Signet of Courage"},
		{LangKey::BuffDolyakStance, "Dolyak Stance"},
		{LangKey::BuffGriffonStance, "Griffon Stance"},
		{LangKey::BuffMoaStance, "Moa Stance"},
		{LangKey::BuffVultureStance, "Vulture Stance"},
		{LangKey::BuffBearStance, "Bear Stance"},
		{LangKey::BuffOneWolfPack, "One Wolf Pack"},
		{LangKey::BuffSoulcleavesSummit, "Soulcleave's Summit"},
		{LangKey::BuffRazorclawsRage, "Razorclaw's Rage"},
		{LangKey::BuffBreakrazorsBastion, "Breakrazor's Bastion"},
		{LangKey::BuffChaosAura, "Chaos Aura"},
		{LangKey::BuffDarkAura, "Dark Aura"},
		{LangKey::BuffFireAura, "Fire Aura"},
		{LangKey::BuffFrostAura, "Frost Aura"},
		{LangKey::BuffLightAura, "Light Aura"},
		{LangKey::BuffMagneticAura, "Magnetic Aura"},
		{LangKey::BuffShockingAura, "Shocking Aura"},
		{LangKey::BuffSoulBarbs, "Soul Barbs"},
		{LangKey::BuffSpectralArmor, "Spectral Armor"},
		{LangKey::BuffInspiringVirtue, "Inspiring Virtue"},
		{LangKey::BuffEternalOasis, "Eternal Oasis"},
		{LangKey::BuffUnbrokenLines, "Unbroken Lines"},
		{LangKey::BuffRiteOfTheGreatDwarf, "Rite of the Great Dwarf"},
		{LangKey::BuffBulwarkGyro, "Bulwark Gyro"},
		{LangKey::BuffSymbolicAvenger, "Symbolic Avenger"},
		{LangKey::BuffInvigoratedBulwark, "Invigorated Bulwark"},
		{LangKey::BuffStickingTogether, "Sticking Together"},
		{LangKey::BuffRigorousCertainty, "Rigorous Certainty"},
		{LangKey::PositionManual, "manual"},
		{LangKey::PositionScreenRelative, "screen relative position"},
		{LangKey::PositionWindowRelative, "window relative position"},
		{LangKey::CornerPositionTopLeft, "top-left"},
		{LangKey::CornerPositionTopRight, "top-right"},
		{LangKey::CornerPositionBottomLeft, "bottom-left"},
		{LangKey::CornerPositionBottomRight, "bottom-right"},
		{LangKey::UpdateWindowHeader, "Arcdps Boon Table Plugin Update"},
		{LangKey::UpdateDesc, "A new update for the Boon Table plugin is available."},
		{LangKey::UpdateCurrentVersion, "Current version"},
		{LangKey::UpdateNewVersion, "New version"},
		{LangKey::UpdateOpenPage, "Open download page"},
		{LangKey::UpdateAutoButton, "Update autmatically"},
		{LangKey::UpdateInProgress, "Autoupdate in progress"},
		{LangKey::UpdateRestartPending, "Autoupdate finished, restart your game to activate it."},
		{LangKey::UpdateError, "Autoupdate error, please update manually."},
		{LangKey::SettingsPosition, "Position"},
		{LangKey::SettingsFromAnchorPanelCorner, "from anchor panel corner"},
		{LangKey::SettingsToThisPanelCorner, "to this panel corner"},
		{LangKey::SettingsFromWindowName, "from window"},
		{LangKey::SettingsDisplay, "Display"},
		{LangKey::SettingsMaxDisplayed, "max displayed"},
		{LangKey::SettingsMaxDisplayedPopup, "The amount of rows, that are displayed.\nTotals and Subgroups also count as row.\nIf you want a squad plus totals, you have to set this to '11'"},
		{LangKey::Unknown, "Unknown"},
	};
};

extern Lang lang;

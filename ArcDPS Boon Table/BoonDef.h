#pragma once

#include <magic_enum.hpp>
#include <array>

enum class Boons {
	Might,
	Fury,
	Regeneration,
	Protection,
	Quickness,
	Alacrity,
	Resolution,
	Vigor,
	Stability,
	Aegis,
	Swiftness,
	Resistance,
	EmpowerAllies,
	PinPointDistribution,
	AssassinsPresence,
	Spotter,
	SoothingMist,
	VampiricAura,
	StrengthInNumbers,
	BannerOfStrength,
	BannerOfDiscipline,
	BannerOfTactics,
	BannerOfDefense,
	SpiritFrost,
	SpiritSun,
	SpiritStone,
	SpiritStorm,
	SpiritWater,
	SignetOfInspiration,
	Stealth,
	Superspeed,
	SignetOfResolve,
	SignetBane,
	SignetOfJudgment,
	SignetOfMercy,
	SignetOfWrath,
	SignetOfCourage,
	StanceDolyak,
	StanceGriffon,
	StanceMoa,
	StanceVulture,
	StanceBear,
	OneWolfPack,
	SoulcleavesSummit,
	RazorclawsRage,
	BreakrazorsBastion,
	AuraChaos,
	AuraDark,
	AuraFire,
	AuraFrost,
	AuraLigth,
	AuraMagnetic,
	AuraShocking,
	SoulBarbs,
	SpectralArmor,
	InspiringVirtue,
	EternalOasis,
	UnbrokenLines,
	RiteOfTheGreatDwarf,
	BulwarkGyro,
	SymbolicAvenger,
	InvigoratedBulwark,
	RigorousCertainty,
};

enum class StackType {
	Multi,
	Single,
};

struct BoonDef {
	Boons Boon = Boons::Might;
	StackType StackType = StackType::Single;

	BoonDef() = default;

	constexpr BoonDef(::Boons pBoon, ::StackType pStackType)
		: Boon(pBoon),
		  StackType(pStackType) {}

	BoonDef(const BoonDef& pOther) = default;
	BoonDef(BoonDef&& pOther) noexcept = default;
	BoonDef& operator=(const BoonDef& pOther) = default;
	BoonDef& operator=(BoonDef&& pOther) noexcept = default;
};

// constexpr const std::array<BoonDef, magic_enum::enum_count<Boons>()>& GET_BOON_DEFS();

// The error is not actually there (compile to see the truth)
static inline constexpr std::array<BoonDef, magic_enum::enum_count<Boons>()> BOON_DEFS = std::to_array<BoonDef>({
	{ Boons::Might, StackType::Multi, },
	{ Boons::Fury, StackType::Single, },
	{ Boons::Regeneration, StackType::Single, },
	{ Boons::Protection, StackType::Single, },
	{ Boons::Quickness, StackType::Single, },
	{ Boons::Alacrity, StackType::Single, },
	{ Boons::Resolution, StackType::Single, },
	{ Boons::Vigor, StackType::Single, },
	{ Boons::Stability, StackType::Multi, },
	{ Boons::Aegis, StackType::Single, },
	{ Boons::Swiftness, StackType::Single, },
	{ Boons::Resistance, StackType::Single, },
	{ Boons::EmpowerAllies, StackType::Single, },
	{ Boons::PinPointDistribution, StackType::Single, },
	{ Boons::AssassinsPresence, StackType::Single, },
	{ Boons::Spotter, StackType::Single, },
	{ Boons::SoothingMist, StackType::Single, },
	{ Boons::VampiricAura, StackType::Single, },
	{ Boons::StrengthInNumbers, StackType::Single, },
	{ Boons::BannerOfStrength, StackType::Single, },
	{ Boons::BannerOfDiscipline, StackType::Single, },
	{ Boons::BannerOfTactics, StackType::Single, },
	{ Boons::BannerOfDefense, StackType::Single, },
	{ Boons::SpiritFrost, StackType::Single, },
	{ Boons::SpiritSun, StackType::Single, },
	{ Boons::SpiritStone, StackType::Single, },
	{ Boons::SpiritStorm, StackType::Single, },
	{ Boons::SpiritWater, StackType::Single, },
	{ Boons::SignetOfInspiration, StackType::Single, },
	{ Boons::Stealth, StackType::Single, },
	{ Boons::Superspeed, StackType::Single, },
	{ Boons::SignetOfResolve, StackType::Single, },
	{ Boons::SignetBane, StackType::Single, },
	{ Boons::SignetOfJudgment, StackType::Single, },
	{ Boons::SignetOfMercy, StackType::Single, },
	{ Boons::SignetOfWrath, StackType::Single, },
	{ Boons::SignetOfCourage, StackType::Single, },
	{ Boons::StanceDolyak, StackType::Single, },
	{ Boons::StanceGriffon, StackType::Single, },
	{ Boons::StanceMoa, StackType::Single, },
	{ Boons::StanceVulture, StackType::Single, },
	{ Boons::StanceBear, StackType::Single, },
	{ Boons::OneWolfPack, StackType::Single, },
	{ Boons::SoulcleavesSummit, StackType::Single, },
	{ Boons::RazorclawsRage, StackType::Single, },
	{ Boons::BreakrazorsBastion, StackType::Single, },
	{ Boons::AuraChaos, StackType::Single, },
	{ Boons::AuraDark, StackType::Single, },
	{ Boons::AuraFire, StackType::Single, },
	{ Boons::AuraFrost, StackType::Single, },
	{ Boons::AuraLigth, StackType::Single, },
	{ Boons::AuraMagnetic, StackType::Single, },
	{ Boons::AuraShocking, StackType::Single, },
	{ Boons::SoulBarbs, StackType::Single, },
	{ Boons::SpectralArmor, StackType::Single, },
	{ Boons::InspiringVirtue, StackType::Single, },
	{ Boons::EternalOasis, StackType::Single, },
	{ Boons::UnbrokenLines, StackType::Single, },
	{ Boons::RiteOfTheGreatDwarf, StackType::Single, },
	{ Boons::BulwarkGyro, StackType::Single, },
	{ Boons::SymbolicAvenger, StackType::Single, },
	{ Boons::InvigoratedBulwark, StackType::Single, },
	{ Boons::RigorousCertainty, StackType::Single, },
});

constexpr std::optional<Boons> GetBoonsFromId(uint32_t pId) {
	switch (pId) {
	case 740: return Boons::Might;
	case 725: return Boons::Fury;
	case 718: return Boons::Regeneration;
	case 717: return Boons::Protection;
	case 1187: return Boons::Quickness;
	case 30328: return Boons::Alacrity;
	case 873: return Boons::Resolution;
	case 726: return Boons::Vigor;
	case 1122: return Boons::Stability;
	case 743: return Boons::Aegis;
	case 719: return Boons::Swiftness;
	case 26980: return Boons::Resistance;
	case 14222: return Boons::EmpowerAllies;
	case 38333: return Boons::PinPointDistribution;
	case 26854: return Boons::AssassinsPresence;
	case 14055: return Boons::Spotter;
	case 5587: return Boons::SoothingMist;
	case 30285: return Boons::VampiricAura;
	case 13796: return Boons::StrengthInNumbers;
	case 14417: return Boons::BannerOfStrength;
	case 14449: return Boons::BannerOfDiscipline;
	case 14450: return Boons::BannerOfTactics;
	case 14543: return Boons::BannerOfDefense;
	case 50421: return Boons::SpiritFrost;
	case 50413: return Boons::SpiritSun;
	case 50415: return Boons::SpiritStone;
	case 50381: return Boons::SpiritStorm;
	case 50386: return Boons::SpiritWater;
	case 10235: return Boons::SignetOfInspiration;
	case 13017: [[fallthrough]];
	case 26142: [[fallthrough]];
	case 10269: return Boons::Stealth;
	case 5974: return Boons::Superspeed;
	case 9220: [[fallthrough]];
	case 46554: return Boons::SignetOfResolve;
	case 9029: [[fallthrough]];
	case 9240: return Boons::SignetBane;
	case 9156: [[fallthrough]];
	case 9239: return Boons::SignetOfJudgment;
	case 9162: [[fallthrough]];
	case 9238: return Boons::SignetOfMercy;
	case 9100: [[fallthrough]];
	case 9237: return Boons::SignetOfWrath;
	case 29633: [[fallthrough]];
	case 43487: return Boons::SignetOfCourage;
	case 41815: return Boons::StanceDolyak;
	case 46280: return Boons::StanceGriffon;
	case 45038: return Boons::StanceMoa;
	case 44651: return Boons::StanceVulture;
	case 40045: return Boons::StanceBear;
	case 44139: return Boons::OneWolfPack;
	case 45026: return Boons::SoulcleavesSummit;
	case 41016: return Boons::RazorclawsRage;
	case 44682: return Boons::BreakrazorsBastion;
	case 10332: return Boons::AuraChaos;
	case 39978: return Boons::AuraDark;
	case 5677: return Boons::AuraFire;
	case 5579: return Boons::AuraFrost;
	case 25518: return Boons::AuraLigth;
	case 5684: return Boons::AuraMagnetic;
	case 5577: return Boons::AuraShocking;
	case 53489: return Boons::SoulBarbs;
	case 10582: return Boons::SpectralArmor;
	case 59592: return Boons::InspiringVirtue;
	case 44871: return Boons::EternalOasis;
	case 43194: return Boons::UnbrokenLines;
	case 26596: [[fallthrough]];
	case 33330: return Boons::RiteOfTheGreatDwarf;
	case 31229: [[fallthrough]];
	case 46910: return Boons::BulwarkGyro;
	case 56890: return Boons::SymbolicAvenger;
	case 30207: return Boons::InvigoratedBulwark;
	case 33652: return Boons::RigorousCertainty;
	default: return std::nullopt;
	}
}

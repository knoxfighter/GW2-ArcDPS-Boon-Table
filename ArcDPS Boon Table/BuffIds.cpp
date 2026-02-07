#include "BuffIds.h"

#include "Helpers.h"
#include "Lang.h"
#include "resource.h"

using ArcdpsExtension::Localization;

std::vector<BoonDef> tracked_buffs;
std::shared_ptr<BoonDef> above90BoonDef;

void init_tracked_buffs() {
	// Gereral Buffs
    tracked_buffs.emplace_back(std::vector<uint32_t>{740}, BT_BuffMight, StackingType_intensity, true, BoonType_boon, ID_Might);
    tracked_buffs.emplace_back(std::vector<uint32_t>{725}, BT_BuffFury, StackingType_duration, true, BoonType_boon, ID_Fury);
    tracked_buffs.emplace_back(std::vector<uint32_t>{718}, BT_BuffRegeneration, StackingType_duration, false, BoonType_boon, ID_Regeneration);
    tracked_buffs.emplace_back(std::vector<uint32_t>{717}, BT_BuffProtection, StackingType_duration, true, BoonType_boon, ID_Protection);
    tracked_buffs.emplace_back(std::vector<uint32_t>{1187}, BT_BuffQuickness, StackingType_duration, true, BoonType_boon, ID_Quickness);
    tracked_buffs.emplace_back(std::vector<uint32_t>{30328},BT_BuffAlacrity, StackingType_duration, true, BoonType_boon, ID_Alacrity);
    tracked_buffs.emplace_back(std::vector<uint32_t>{873}, BT_BuffResolution, StackingType_duration, false, BoonType_boon, ID_Resolution);
    tracked_buffs.emplace_back(std::vector<uint32_t>{726}, BT_BuffVigor, StackingType_duration, false, BoonType_boon, ID_Vigor);
    tracked_buffs.emplace_back(std::vector<uint32_t>{1122}, BT_BuffStability, StackingType_intensity, false, BoonType_boon, ID_Stability);
    tracked_buffs.emplace_back(std::vector<uint32_t>{743}, BT_BuffAegis, StackingType_duration, false, BoonType_boon, ID_Aegis);
    tracked_buffs.emplace_back(std::vector<uint32_t>{719}, BT_BuffSwiftness, StackingType_duration, false, BoonType_boon, ID_Swiftness);
    tracked_buffs.emplace_back(std::vector<uint32_t>{26980}, BT_BuffResistance, StackingType_duration, false, BoonType_boon, ID_Resistance);

    tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Empowered Allies (legacy)"
    tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Pinpoint Distribution (legacy)"
    tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Assassin's Presence (legacy)"
    tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Spotter (legacy)"
	tracked_buffs.emplace_back(std::vector<uint32_t>{5587}, BT_BuffSoothingMist, StackingType_single, false, BoonType_Elementalist, ID_Soothing_Mist);
	tracked_buffs.emplace_back(std::vector<uint32_t>{30285}, BT_BuffVampiricAura, StackingType_single, false, BoonType_Necromancer, ID_Vampiric_Presence);
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Strength in Numbers (legacy)"
	// warrior banners
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Banner of Strength (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Banner of Discipline (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Banner of Tactics (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Banner of Defense (legacy)"
	// ranger spirits
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Frost Spirit (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Sun Spirit (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Stone Spirit (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Storm Spirit (legacy)"
	tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Water Spirit (legacy)"
    // general
    tracked_buffs.emplace_back(std::vector<uint32_t>{10235}, BT_BuffSignetInspiration, StackingType_single, false, BoonType_Mesmer, ID_Signet_of_Inspiration2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{10269, 13017, 26142, 32747, 58026}, BT_BuffStealth, StackingType_duration, false, BoonType_other, ID_Stealth2); //stealth + Hide in Shadows
    tracked_buffs.emplace_back(std::vector<uint32_t>{5974}, BT_BuffSuperspeed, StackingType_single, false, BoonType_other, ID_Super_Speed2);
    // Guardian Signets
    tracked_buffs.emplace_back(std::vector<uint32_t>{9220, 46554}, BT_BuffSignetResolve, StackingType_single, false, BoonType_Guardian, ID_Signet_Resolve);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9029, 9240}, BT_BuffSignetBane, StackingType_single, false, BoonType_Guardian, ID_Signet_Bane);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9156, 9239}, BT_BuffSignetJudgment, StackingType_single, false, BoonType_Guardian, ID_Signet_Judgment);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9162, 9238}, BT_BuffSignetMercy, StackingType_single, false, BoonType_Guardian, ID_Signet_Mercy);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9100, 9237}, BT_BuffSignetWrath, StackingType_single, false, BoonType_Guardian, ID_Signet_Wrath);
    tracked_buffs.emplace_back(std::vector<uint32_t>{29633, 43487}, BT_BuffSignetCourage, StackingType_single, false, BoonType_Guardian, ID_Signet_Courage);
    // Souelbeast stances
    tracked_buffs.emplace_back(std::vector<uint32_t>{41815}, BT_BuffDolyakStance, StackingType_duration, false, BoonType_Ranger, ID_Dolyak_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{46280}, BT_BuffGriffonStance, StackingType_duration, false, BoonType_Ranger, ID_Griffon_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{45038}, BT_BuffMoaStance, StackingType_duration, false, BoonType_Ranger, ID_Moa_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44651}, BT_BuffVultureStance, StackingType_duration, false, BoonType_Ranger, ID_Vulture_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{40045}, BT_BuffBearStance, StackingType_duration, false, BoonType_Ranger, ID_Bear_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44139}, BT_BuffOneWolfPack, StackingType_duration, false, BoonType_Ranger, ID_One_Wolf_Pack);
    // Revenant
    tracked_buffs.emplace_back(std::vector<uint32_t>{45026}, BT_BuffSoulcleavesSummit, StackingType_single, false, BoonType_Revenant, ID_Soulcleaves_Summit);
    tracked_buffs.emplace_back(std::vector<uint32_t>{41016}, BT_BuffRazorclawsRage, StackingType_single, false, BoonType_Revenant, ID_Razorclaws_Rage);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44682}, BT_BuffBreakrazorsBastion, StackingType_single, false, BoonType_Revenant, ID_Breakrazors_Bastion);
    // Aura
    tracked_buffs.emplace_back(std::vector<uint32_t>{10332}, BT_BuffChaosAura, StackingType_single, false, BoonType_Aura, ID_Chaos_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{39978}, BT_BuffDarkAura, StackingType_single, false, BoonType_Aura, ID_Dark_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5677}, BT_BuffFireAura, StackingType_single, false, BoonType_Aura, ID_Fire_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5579}, BT_BuffFrostAura, StackingType_single, false, BoonType_Aura, ID_Frost_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{25518, 68927}, BT_BuffLightAura, StackingType_single, false, BoonType_Aura, ID_Light_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5684}, BT_BuffMagneticAura, StackingType_single, false, BoonType_Aura, ID_Magnetic_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5577}, BT_BuffShockingAura, StackingType_single, false, BoonType_Aura, ID_Shocking_Aura);

    tracked_buffs.emplace_back(std::vector<uint32_t>{53489}, BT_BuffSoulBarbs, StackingType_duration, false, BoonType_Necromancer, ID_Soul_Barbs);
    tracked_buffs.emplace_back(std::vector<uint32_t>{10582}, BT_BuffSpectralArmor, StackingType_single, false, BoonType_Necromancer, ID_Spectral_Armor);
    tracked_buffs.emplace_back(std::vector<uint32_t>{59592}, BT_BuffInspiringVirtue, StackingType_single, false, BoonType_Guardian, ID_Inspiring_Virtue);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44871}, BT_BuffEternalOasis, StackingType_single, false, BoonType_Guardian, ID_Eternal_Oasis);
    tracked_buffs.emplace_back(std::vector<uint32_t>{43194}, BT_BuffUnbrokenLines, StackingType_single, false, BoonType_Guardian, ID_Unbroken_Lines);
	tracked_buffs.emplace_back(std::vector<uint32_t>{26596, 33330}, BT_BuffRiteOfTheGreatDwarf, StackingType_single, false, BoonType_Revenant, ID_Rite_of_the_Great_Dwarf);
    tracked_buffs.emplace_back(ArcdpsExtension::ET_Unknown); // "Bulwark Gyro (legacy)"
	tracked_buffs.emplace_back(std::vector<uint32_t>{56890}, BT_BuffSymbolicAvenger, StackingType_single, false, BoonType_Guardian, ID_Symbolic_Avenger);
	tracked_buffs.emplace_back(std::vector<uint32_t>{30207}, BT_BuffInvigoratedBulwark, StackingType_single, false, BoonType_Guardian, ID_Invigorated_Bulwark);
	tracked_buffs.emplace_back(std::vector<uint32_t>{33652}, BT_BuffRigorousCertainty, StackingType_single, false, BoonType_other, ID_Rigorous_Certainty);

    // Relics
    tracked_buffs.emplace_back(std::vector<uint32_t>{69795}, BT_BuffRelicAristocracy, StackingType_intensity, false, BoonType_Relic, ID_Relic_Aristocracy);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71132}, BT_BuffRelicMonk, StackingType_intensity, false, BoonType_Relic, ID_Relic_Monk);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70913}, BT_BuffRelicBrawler, StackingType_single, false, BoonType_Relic, ID_Relic_Brawler);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70767}, BT_BuffRelicThief, StackingType_intensity, false, BoonType_Relic, ID_Relic_Thief);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69855}, BT_BuffRelicFireworks, StackingType_single, false, BoonType_Relic, ID_Relic_Fireworks);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70839}, BT_BuffRelicDaredevil, StackingType_single, false, BoonType_Relic, ID_Relic_Daredevil);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70282}, BT_BuffRelicDeadeye, StackingType_single, false, BoonType_Relic, ID_Relic_Deadeye);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71217}, BT_BuffRelicFirebrand, StackingType_single, false, BoonType_Relic, ID_Relic_Firebrand);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69606}, BT_BuffRelicHerald, StackingType_intensity, false, BoonType_Relic, ID_Relic_Herald);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71217}, BT_BuffRelicScourge, StackingType_single, false, BoonType_Relic, ID_Relic_Scourge);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70390}, BT_BuffRelicWeaver, StackingType_single, false, BoonType_Relic, ID_Relic_Weaver);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70460}, BT_BuffRelicZephyrite, StackingType_single, false, BoonType_Relic, ID_Relic_Zephyrite);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70353}, BT_BuffRelicLyhr, StackingType_single, false, BoonType_Relic, ID_Relic_Lyhr);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69620}, BT_BuffRelicMabon, StackingType_intensity, false, BoonType_Relic, ID_Relic_Mabon);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69961}, BT_BuffRelicVass, StackingType_intensity, false, BoonType_Relic, ID_Relic_Vass);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71431}, BT_BuffRelicNourys, StackingType_single, false, BoonType_Relic, ID_Relic_Nourys);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73455}, BT_BuffRelicStormsinger, StackingType_single, false, BoonType_Relic, ID_Relic_Stormsinger);
    tracked_buffs.emplace_back(std::vector<uint32_t>{74410}, BT_BuffRelicSorrow, StackingType_single, false, BoonType_Relic, ID_Relic_Sorrow);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73181}, BT_BuffRelicBlightbringer, StackingType_single, false, BoonType_Relic, ID_Relic_Blightbringer);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73955}, BT_BuffRelicClaw, StackingType_single, false, BoonType_Relic, ID_Relic_Claw);
    tracked_buffs.emplace_back(std::vector<uint32_t>{74793}, BT_BuffRelicMountBalrior, StackingType_single, false, BoonType_Relic, ID_Relic_MountBalrior);
    tracked_buffs.emplace_back(std::vector<uint32_t>{75432}, BT_BuffRelicThorns, StackingType_intensity, false, BoonType_Relic, ID_Relic_Thorns, 10);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76372}, BT_BuffRelicTitanicPotential, StackingType_intensity, false, BoonType_Relic, ID_Relic_TitanicPotential);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76351}, BT_BuffRelicSoulOfTheTitan, StackingType_single, false, BoonType_Relic, ID_Relic_SoulOfTheTitan);
    tracked_buffs.emplace_back(std::vector<uint32_t>{104800}, BT_BuffRelicBloodstoneVolatility, StackingType_intensity, false, BoonType_Relic, ID_Relic_Bloodstone);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76326}, BT_BuffRelicBloodstoneFervor, StackingType_single, false, BoonType_Relic, ID_Relic_Bloodstone);
	
	// above 90
	above90BoonDef = std::make_shared<BoonDef>(std::vector<uint32_t>{static_cast<uint32_t>(-1)}, BT_Above90Hp, StackingType_single, false, BoonType_other, ID_Writ_of_Masterful_Strength); // above 90% hp (e.g. scholar)

    auto& iconLoader = ArcdpsExtension::IconLoader::instance();

    size_t iconTextureId = 0;

    for (auto& tracked_buff : tracked_buffs)
    {
        tracked_buff.iconTextureId = ++iconTextureId;
        iconLoader.RegisterResource(tracked_buff.iconTextureId, tracked_buff.icon);
    }

    above90BoonDef->iconTextureId = ++iconTextureId;
    iconLoader.RegisterResource(above90BoonDef->iconTextureId, above90BoonDef->icon);
}

BoonDef* getTrackedBoon(uint32_t new_id) {
	for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff) {
		for (auto current_id = current_buff->ids.cbegin(); current_id != current_buff->ids.cend(); ++current_id) {
			if (*current_id == new_id) return &*current_buff;
		}
	}
	return nullptr;
}

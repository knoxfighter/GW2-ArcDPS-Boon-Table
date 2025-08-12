#include "BuffIds.h"

#include "Helpers.h"
#include "Lang.h"
#include "resource.h"

std::vector<BoonDef> tracked_buffs;
std::shared_ptr<BoonDef> above90BoonDef;

void init_tracked_buffs() {
	// Gereral Buffs
    tracked_buffs.emplace_back(std::vector<uint32_t>{740}, lang.translate(LangKey::BuffMight), StackingType_intensity, true, BoonType_boon, ID_Might);
    tracked_buffs.emplace_back(std::vector<uint32_t>{725}, lang.translate(LangKey::BuffFury), StackingType_duration, true, BoonType_boon, ID_Fury);
    tracked_buffs.emplace_back(std::vector<uint32_t>{718}, lang.translate(LangKey::BuffRegeneration), StackingType_duration, false, BoonType_boon, ID_Regeneration);
    tracked_buffs.emplace_back(std::vector<uint32_t>{717}, lang.translate(LangKey::BuffProtection), StackingType_duration, true, BoonType_boon, ID_Protection);
    tracked_buffs.emplace_back(std::vector<uint32_t>{1187}, lang.translate(LangKey::BuffQuickness), StackingType_duration, true, BoonType_boon, ID_Quickness);
    tracked_buffs.emplace_back(std::vector<uint32_t>{30328}, lang.translate(LangKey::BuffAlacrity), StackingType_duration, true, BoonType_boon, ID_Alacrity);
    tracked_buffs.emplace_back(std::vector<uint32_t>{873}, lang.translate(LangKey::BuffResolution), StackingType_duration, false, BoonType_boon, ID_Resolution);
    tracked_buffs.emplace_back(std::vector<uint32_t>{726}, lang.translate(LangKey::BuffVigor), StackingType_duration, false, BoonType_boon, ID_Vigor);
    tracked_buffs.emplace_back(std::vector<uint32_t>{1122}, lang.translate(LangKey::BuffStability), StackingType_intensity, false, BoonType_boon, ID_Stability);
    tracked_buffs.emplace_back(std::vector<uint32_t>{743}, lang.translate(LangKey::BuffAegis), StackingType_duration, false, BoonType_boon, ID_Aegis);
    tracked_buffs.emplace_back(std::vector<uint32_t>{719}, lang.translate(LangKey::BuffSwiftness), StackingType_duration, false, BoonType_boon, ID_Swiftness);
    tracked_buffs.emplace_back(std::vector<uint32_t>{26980}, lang.translate(LangKey::BuffResistance), StackingType_duration, false, BoonType_boon, ID_Resistance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{14222}, lang.translate(LangKey::BuffEmpowerAllies), StackingType_single, false, BoonType_Warrior, ID_Empower_Allies);
    tracked_buffs.emplace_back(std::vector<uint32_t>{38333}, lang.translate(LangKey::BuffPinpointDistribution), StackingType_single, false, BoonType_Engineer, ID_Pinpoint_Distribution);
    tracked_buffs.emplace_back(std::vector<uint32_t>{26854}, lang.translate(LangKey::BuffAssassinsPresence), StackingType_single, false, BoonType_Revenant, ID_Assassins_Presence2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{14055}, lang.translate(LangKey::BuffSpotter), StackingType_single, false, BoonType_Ranger, ID_Spotter);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5587}, lang.translate(LangKey::BuffSoothingMist), StackingType_single, false, BoonType_Elementalist, ID_Soothing_Mist);
    tracked_buffs.emplace_back(std::vector<uint32_t>{30285}, lang.translate(LangKey::BuffVampiricAura), StackingType_single, false, BoonType_Necromancer, ID_Vampiric_Presence);
    tracked_buffs.emplace_back(std::vector<uint32_t>{13796}, lang.translate(LangKey::BuffStrengthInNumbers), StackingType_single, false, BoonType_Guardian, ID_Strength_in_Numbers);
    // warrior banners
    tracked_buffs.emplace_back(std::vector<uint32_t>{14417}, lang.translate(LangKey::BuffStrength) /*Banner of Strength*/, StackingType_single, false, BoonType_Warrior, ID_Banner_of_Strength2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{14449}, lang.translate(LangKey::BuffBannerDiscipline), StackingType_single, false, BoonType_Warrior, ID_Banner_of_Discipline2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{14450}, lang.translate(LangKey::BuffBannerTactics), StackingType_single, false, BoonType_Warrior, ID_Banner_of_Tactics2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{14543}, lang.translate(LangKey::BuffBannerDefense), StackingType_single, false, BoonType_Warrior, ID_Banner_of_Defense2);
    // ranger spirits
    tracked_buffs.emplace_back(std::vector<uint32_t>{50421}, lang.translate(LangKey::BuffSpiritFrost), StackingType_single, false, BoonType_Ranger, ID_Frost_Spirit2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{50413}, lang.translate(LangKey::BuffSpiritSun), StackingType_single, false, BoonType_Ranger, ID_Sun_Spirit2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{50415}, lang.translate(LangKey::BuffSpiritStone), StackingType_single, false, BoonType_Ranger, ID_Stone_Spirit2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{50381}, lang.translate(LangKey::BuffSpiritStorm), StackingType_single, false, BoonType_Ranger, ID_Storm_Spirit2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{50386}, lang.translate(LangKey::BuffSpiritWater), StackingType_single, false, BoonType_Ranger, ID_Water_Spirit);
    // general
    tracked_buffs.emplace_back(std::vector<uint32_t>{10235}, lang.translate(LangKey::BuffSignetInspiration), StackingType_single, false, BoonType_Mesmer, ID_Signet_of_Inspiration2);
    tracked_buffs.emplace_back(std::vector<uint32_t>{13017, 26142, 10269 }, lang.translate(LangKey::BuffStealth), StackingType_duration, false, BoonType_other, ID_Stealth2); //stealth + Hide in Shadows 
    tracked_buffs.emplace_back(std::vector<uint32_t>{5974}, lang.translate(LangKey::BuffSuperspeed), StackingType_single, false, BoonType_other, ID_Super_Speed2);
    // Guardian Signets
    tracked_buffs.emplace_back(std::vector<uint32_t>{9220, 46554}, lang.translate(LangKey::BuffSignetResolve), StackingType_single, false, BoonType_Guardian, ID_Signet_Resolve);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9029, 9240}, lang.translate(LangKey::BuffSignetBane), StackingType_single, false, BoonType_Guardian, ID_Signet_Bane);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9156, 9239}, lang.translate(LangKey::BuffSignetJudgment), StackingType_single, false, BoonType_Guardian, ID_Signet_Judgment);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9162, 9238}, lang.translate(LangKey::BuffSignetMercy), StackingType_single, false, BoonType_Guardian, ID_Signet_Mercy);
    tracked_buffs.emplace_back(std::vector<uint32_t>{9100, 9237}, lang.translate(LangKey::BuffSignetWrath), StackingType_single, false, BoonType_Guardian, ID_Signet_Wrath);
    tracked_buffs.emplace_back(std::vector<uint32_t>{29633, 43487}, lang.translate(LangKey::BuffSignetCourage), StackingType_single, false, BoonType_Guardian, ID_Signet_Courage);
    // Souelbeast stances
    tracked_buffs.emplace_back(std::vector<uint32_t>{41815}, lang.translate(LangKey::BuffDolyakStance), StackingType_duration, false, BoonType_Ranger, ID_Dolyak_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{46280}, lang.translate(LangKey::BuffGriffonStance), StackingType_duration, false, BoonType_Ranger, ID_Griffon_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{45038}, lang.translate(LangKey::BuffMoaStance), StackingType_duration, false, BoonType_Ranger, ID_Moa_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44651}, lang.translate(LangKey::BuffVultureStance), StackingType_duration, false, BoonType_Ranger, ID_Vulture_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{40045}, lang.translate(LangKey::BuffBearStance), StackingType_duration, false, BoonType_Ranger, ID_Bear_Stance);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44139}, lang.translate(LangKey::BuffOneWolfPack), StackingType_duration, false, BoonType_Ranger, ID_One_Wolf_Pack);
    // Revenant
    tracked_buffs.emplace_back(std::vector<uint32_t>{45026}, lang.translate(LangKey::BuffSoulcleavesSummit), StackingType_single, false, BoonType_Revenant, ID_Soulcleaves_Summit);
    tracked_buffs.emplace_back(std::vector<uint32_t>{41016}, lang.translate(LangKey::BuffRazorclawsRage), StackingType_single, false, BoonType_Revenant, ID_Razorclaws_Rage);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44682}, lang.translate(LangKey::BuffBreakrazorsBastion), StackingType_single, false, BoonType_Revenant, ID_Breakrazors_Bastion);
    // Aura
    tracked_buffs.emplace_back(std::vector<uint32_t>{10332}, lang.translate(LangKey::BuffChaosAura), StackingType_single, false, BoonType_Aura, ID_Chaos_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{39978}, lang.translate(LangKey::BuffDarkAura), StackingType_single, false, BoonType_Aura, ID_Dark_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5677}, lang.translate(LangKey::BuffFireAura), StackingType_single, false, BoonType_Aura, ID_Fire_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5579}, lang.translate(LangKey::BuffFrostAura), StackingType_single, false, BoonType_Aura, ID_Frost_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{25518}, lang.translate(LangKey::BuffLightAura), StackingType_single, false, BoonType_Aura, ID_Light_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5684}, lang.translate(LangKey::BuffMagneticAura), StackingType_single, false, BoonType_Aura, ID_Magnetic_Aura);
    tracked_buffs.emplace_back(std::vector<uint32_t>{5577}, lang.translate(LangKey::BuffShockingAura), StackingType_single, false, BoonType_Aura, ID_Shocking_Aura);

    tracked_buffs.emplace_back(std::vector<uint32_t>{53489}, lang.translate(LangKey::BuffSoulBarbs), StackingType_duration, false, BoonType_Necromancer, ID_Soul_Barbs);
    tracked_buffs.emplace_back(std::vector<uint32_t>{10582}, lang.translate(LangKey::BuffSpectralArmor), StackingType_single, false, BoonType_Necromancer, ID_Spectral_Armor);
    tracked_buffs.emplace_back(std::vector<uint32_t>{59592}, lang.translate(LangKey::BuffInspiringVirtue), StackingType_single, false, BoonType_Guardian, ID_Inspiring_Virtue);
    tracked_buffs.emplace_back(std::vector<uint32_t>{44871}, lang.translate(LangKey::BuffEternalOasis), StackingType_single, false, BoonType_Guardian, ID_Eternal_Oasis);
    tracked_buffs.emplace_back(std::vector<uint32_t>{43194}, lang.translate(LangKey::BuffUnbrokenLines), StackingType_single, false, BoonType_Guardian, ID_Unbroken_Lines);
	tracked_buffs.emplace_back(std::vector<uint32_t>{26596, 33330}, lang.translate(LangKey::BuffRiteOfTheGreatDwarf), StackingType_single, false, BoonType_Revenant, ID_Rite_of_the_Great_Dwarf);
	tracked_buffs.emplace_back(std::vector<uint32_t>{31229, 46910}, lang.translate(LangKey::BuffBulwarkGyro), StackingType_single, false, BoonType_Engineer, ID_Bulwark_Gyro);
	tracked_buffs.emplace_back(std::vector<uint32_t>{56890}, lang.translate(LangKey::BuffSymbolicAvenger), StackingType_single, false, BoonType_Guardian, ID_Symbolic_Avenger);
	tracked_buffs.emplace_back(std::vector<uint32_t>{30207}, lang.translate(LangKey::BuffInvigoratedBulwark), StackingType_single, false, BoonType_Guardian, ID_Invigorated_Bulwark);
	// tracked_buffs.emplace_back(std::vector<uint32_t>{53932}, lang.translate(LangKey::BuffStickingTogether), StackingType_single, false, BoonType_other, new Icon(ID_Sticking_Together, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{33652}, lang.translate(LangKey::BuffRigorousCertainty), StackingType_single, false, BoonType_other, ID_Rigorous_Certainty);

    // Relics
    tracked_buffs.emplace_back(std::vector<uint32_t>{69795}, lang.translate(LangKey::BuffRelicAristocracy), StackingType_intensity, false, BoonType_Relic, ID_Relic_Aristocracy);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71132}, lang.translate(LangKey::BuffRelicMonk), StackingType_intensity, false, BoonType_Relic, ID_Relic_Monk);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70913}, lang.translate(LangKey::BuffRelicBrawler), StackingType_single, false, BoonType_Relic, ID_Relic_Brawler);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70767}, lang.translate(LangKey::BuffRelicThief), StackingType_intensity, false, BoonType_Relic, ID_Relic_Thief);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69855}, lang.translate(LangKey::BuffRelicFireworks), StackingType_single, false, BoonType_Relic, ID_Relic_Fireworks);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70839}, lang.translate(LangKey::BuffRelicDaredevil), StackingType_single, false, BoonType_Relic, ID_Relic_Daredevil);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70282}, lang.translate(LangKey::BuffRelicDeadeye), StackingType_single, false, BoonType_Relic, ID_Relic_Deadeye);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71217}, lang.translate(LangKey::BuffRelicFirebrand), StackingType_single, false, BoonType_Relic, ID_Relic_Firebrand);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69606}, lang.translate(LangKey::BuffRelicHerald), StackingType_intensity, false, BoonType_Relic, ID_Relic_Herald);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71217}, lang.translate(LangKey::BuffRelicScourge), StackingType_single, false, BoonType_Relic, ID_Relic_Scourge);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70390}, lang.translate(LangKey::BuffRelicWeaver), StackingType_single, false, BoonType_Relic, ID_Relic_Weaver);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70460}, lang.translate(LangKey::BuffRelicZephyrite), StackingType_single, false, BoonType_Relic, ID_Relic_Zephyrite);
    tracked_buffs.emplace_back(std::vector<uint32_t>{70353}, lang.translate(LangKey::BuffRelicLyhr), StackingType_single, false, BoonType_Relic, ID_Relic_Lyhr);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69620}, lang.translate(LangKey::BuffRelicMabon), StackingType_intensity, false, BoonType_Relic, ID_Relic_Mabon);
    tracked_buffs.emplace_back(std::vector<uint32_t>{69961}, lang.translate(LangKey::BuffRelicVass), StackingType_intensity, false, BoonType_Relic, ID_Relic_Vass);
    tracked_buffs.emplace_back(std::vector<uint32_t>{71431}, lang.translate(LangKey::BuffRelicNourys), StackingType_single, false, BoonType_Relic, ID_Relic_Nourys);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73455}, lang.translate(LangKey::BuffRelicStormsinger), StackingType_single, false, BoonType_Relic, ID_Relic_Stormsinger);
    tracked_buffs.emplace_back(std::vector<uint32_t>{74410}, lang.translate(LangKey::BuffRelicSorrow), StackingType_single, false, BoonType_Relic, ID_Relic_Sorrow);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73181}, lang.translate(LangKey::BuffRelicBlightbringer), StackingType_single, false, BoonType_Relic, ID_Relic_Blightbringer);
    tracked_buffs.emplace_back(std::vector<uint32_t>{73955}, lang.translate(LangKey::BuffRelicClaw), StackingType_single, false, BoonType_Relic, ID_Relic_Claw);
    tracked_buffs.emplace_back(std::vector<uint32_t>{74793}, lang.translate(LangKey::BuffRelicMountBalrior), StackingType_single, false, BoonType_Relic, ID_Relic_MountBalrior);
    tracked_buffs.emplace_back(std::vector<uint32_t>{75432}, lang.translate(LangKey::BuffRelicThorns), StackingType_single, false, BoonType_Relic, ID_Relic_Thorns);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76372}, lang.translate(LangKey::BuffRelicTitanicPotential), StackingType_intensity, false, BoonType_Relic, ID_Relic_TitanicPotential);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76351}, lang.translate(LangKey::BuffRelicSoulOfTheTitan), StackingType_single, false, BoonType_Relic, ID_Relic_SoulOfTheTitan);
    tracked_buffs.emplace_back(std::vector<uint32_t>{104800}, lang.translate(LangKey::BuffRelicBloodstoneVolatility), StackingType_intensity, false, BoonType_Relic, ID_Relic_Bloodstone);
    tracked_buffs.emplace_back(std::vector<uint32_t>{76326}, lang.translate(LangKey::BuffRelicBloodstoneFervor), StackingType_single, false, BoonType_Relic, ID_Relic_Bloodstone);
	
	// above 90
	above90BoonDef = std::make_shared<BoonDef>(std::vector<uint32_t>{static_cast<uint32_t>(-1)}, lang.translate(LangKey::Above90Hp), StackingType_single, false, BoonType_other, ID_Rune_Scholar); // above 90% hp (e.g. scholar)
}

BoonDef* getTrackedBoon(uint32_t new_id) {
	for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff) {
		for (auto current_id = current_buff->ids.cbegin(); current_id != current_buff->ids.cend(); ++current_id) {
			if (*current_id == new_id) return &*current_buff;
		}
	}
	return nullptr;
}

BoonDef::BoonDef(std::vector<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category, UINT new_icon)
: ids(new_ids), name(new_name), stacking_type(new_stacking_type), is_relevant(new_is_relevant), category(new_category), icon(new_icon) {
}

#include "BuffIds.h"

#include "Helpers.h"
#include "Lang.h"
#include "resource.h"

std::vector<BoonDef> tracked_buffs;
std::shared_ptr<BoonDef> above90BoonDef;

void init_tracked_buffs(IDirect3DDevice9* d3d9device) {
	// Gereral Buffs
	tracked_buffs.emplace_back(std::vector<uint32_t>{740}, lang.translate(LangKey::BuffMight), StackingType_intensity, true, BoonType_boon, new Icon(ID_Might, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{725}, lang.translate(LangKey::BuffFury), StackingType_duration, true, BoonType_boon, new Icon(ID_Fury, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{718}, lang.translate(LangKey::BuffRegeneration), StackingType_duration, false, BoonType_boon, new Icon(ID_Regeneration, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{717}, lang.translate(LangKey::BuffProtection), StackingType_duration, true, BoonType_boon, new Icon(ID_Protection, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{1187}, lang.translate(LangKey::BuffQuickness), StackingType_duration, true, BoonType_boon, new Icon(ID_Quickness, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{30328}, lang.translate(LangKey::BuffAlacrity), StackingType_duration, true, BoonType_boon, new Icon(ID_Alacrity, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{873}, lang.translate(LangKey::BuffRetaliation), StackingType_duration, false, BoonType_boon, new Icon(ID_Retaliation, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{726}, lang.translate(LangKey::BuffVigor), StackingType_duration, false, BoonType_boon, new Icon(ID_Vigor, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{1122}, lang.translate(LangKey::BuffStability), StackingType_intensity, false, BoonType_boon, new Icon(ID_Stability, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{743}, lang.translate(LangKey::BuffAegis), StackingType_duration, false, BoonType_boon, new Icon(ID_Aegis, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{719}, lang.translate(LangKey::BuffSwiftness), StackingType_duration, false, BoonType_boon, new Icon(ID_Swiftness, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{26980}, lang.translate(LangKey::BuffResistance), StackingType_duration, false, BoonType_boon, new Icon(ID_Resistance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14222}, lang.translate(LangKey::BuffEmpowerAllies), StackingType_single, false, BoonType_Warrior, new Icon(ID_Empower_Allies, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{38333}, lang.translate(LangKey::BuffPinpointDistribution), StackingType_single, false, BoonType_Engineer, new Icon(ID_Pinpoint_Distribution, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{26854}, lang.translate(LangKey::BuffAssassinsPresence), StackingType_single, false, BoonType_Revenant, new Icon(ID_Assassins_Presence2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14055}, lang.translate(LangKey::BuffSpotter), StackingType_single, false, BoonType_Ranger, new Icon(ID_Spotter, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5587}, lang.translate(LangKey::BuffSoothingMist), StackingType_single, false, BoonType_Elementalist, new Icon(ID_Soothing_Mist, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{30285}, lang.translate(LangKey::BuffVampiricAura), StackingType_single, false, BoonType_Necromancer, new Icon(ID_Vampiric_Presence, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{13796}, lang.translate(LangKey::BuffStrengthInNumbers), StackingType_single, false, BoonType_Guardian, new Icon(ID_Strength_in_Numbers, self_dll, d3d9device));
	// warrior banners
	tracked_buffs.emplace_back(std::vector<uint32_t>{14417}, lang.translate(LangKey::BuffStrength) /*Banner of Strength*/, StackingType_single, false, BoonType_Warrior, new Icon(ID_Banner_of_Strength2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14449}, lang.translate(LangKey::BuffBannerDiscipline), StackingType_single, false, BoonType_Warrior, new Icon(ID_Banner_of_Discipline2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14450}, lang.translate(LangKey::BuffBannerTactics), StackingType_single, false, BoonType_Warrior, new Icon(ID_Banner_of_Tactics2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14543}, lang.translate(LangKey::BuffBannerDefense), StackingType_single, false, BoonType_Warrior, new Icon(ID_Banner_of_Defense2, self_dll, d3d9device));
	// ranger spirits
	tracked_buffs.emplace_back(std::vector<uint32_t>{50421}, lang.translate(LangKey::BuffSpiritFrost), StackingType_single, false, BoonType_Ranger, new Icon(ID_Frost_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50413}, lang.translate(LangKey::BuffSpiritSun), StackingType_single, false, BoonType_Ranger, new Icon(ID_Sun_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50415}, lang.translate(LangKey::BuffSpiritStone), StackingType_single, false, BoonType_Ranger, new Icon(ID_Stone_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50381}, lang.translate(LangKey::BuffSpiritStorm), StackingType_single, false, BoonType_Ranger, new Icon(ID_Storm_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50386}, lang.translate(LangKey::BuffSpiritWater), StackingType_single, false, BoonType_Ranger, new Icon(ID_Water_Spirit, self_dll, d3d9device));
	// general
	tracked_buffs.emplace_back(std::vector<uint32_t>{10235}, lang.translate(LangKey::BuffSignetInspiration), StackingType_single, false, BoonType_Mesmer, new Icon(ID_Signet_of_Inspiration2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{13017, 26142, 10269 }, lang.translate(LangKey::BuffStealth), StackingType_duration, false, BoonType_other, new Icon(ID_Stealth2, self_dll, d3d9device)); //stealth + Hide in Shadows 
	tracked_buffs.emplace_back(std::vector<uint32_t>{5974}, lang.translate(LangKey::BuffSuperspeed), StackingType_single, false, BoonType_other, new Icon(ID_Super_Speed2, self_dll, d3d9device));
	// Guardian Signets
	tracked_buffs.emplace_back(std::vector<uint32_t>{9220, 46554}, lang.translate(LangKey::BuffSignetResolve), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Resolve, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{9029, 9240}, lang.translate(LangKey::BuffSignetBane), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Bane, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{9156, 9239}, lang.translate(LangKey::BuffSignetJudgment), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Judgment, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{9162, 9238}, lang.translate(LangKey::BuffSignetMercy), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Mercy, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{9100, 9237}, lang.translate(LangKey::BuffSignetWrath), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Wrath, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{29633, 43487}, lang.translate(LangKey::BuffSignetCourage), StackingType_single, false, BoonType_Guardian, new Icon(ID_Signet_Courage, self_dll, d3d9device));
	// Souelbeast stances
	tracked_buffs.emplace_back(std::vector<uint32_t>{41815}, lang.translate(LangKey::BuffDolyakStance), StackingType_duration, false, BoonType_Ranger, new Icon(ID_Dolyak_Stance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{46280}, lang.translate(LangKey::BuffGriffonStance), StackingType_duration, false, BoonType_Ranger, new Icon(ID_Griffon_Stance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{45038}, lang.translate(LangKey::BuffMoaStance), StackingType_duration, false, BoonType_Ranger, new Icon(ID_Moa_Stance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{44651}, lang.translate(LangKey::BuffVultureStance), StackingType_duration, false, BoonType_Ranger, new Icon(ID_Vulture_Stance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{40045}, lang.translate(LangKey::BuffBearStance), StackingType_duration, false, BoonType_Ranger, new Icon(ID_Bear_Stance, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{44139}, lang.translate(LangKey::BuffOneWolfPack), StackingType_duration, false, BoonType_Ranger, new Icon(ID_One_Wolf_Pack, self_dll, d3d9device));
	// Revenant
	tracked_buffs.emplace_back(std::vector<uint32_t>{45026}, lang.translate(LangKey::BuffSoulcleavesSummit), StackingType_single, false, BoonType_Revenant, new Icon(ID_Soulcleaves_Summit, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{41016}, lang.translate(LangKey::BuffRazorclawsRage), StackingType_single, false, BoonType_Revenant, new Icon(ID_Razorclaws_Rage, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{44682}, lang.translate(LangKey::BuffBreakrazorsBastion), StackingType_single, false, BoonType_Revenant, new Icon(ID_Breakrazors_Bastion, self_dll, d3d9device));
	// Aura
	tracked_buffs.emplace_back(std::vector<uint32_t>{10332}, lang.translate(LangKey::BuffChaosAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Chaos_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{39978}, lang.translate(LangKey::BuffDarkAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Dark_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5677}, lang.translate(LangKey::BuffFireAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Fire_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5579}, lang.translate(LangKey::BuffFrostAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Frost_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{25518}, lang.translate(LangKey::BuffLightAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Light_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5684}, lang.translate(LangKey::BuffMagneticAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Magnetic_Aura, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5577}, lang.translate(LangKey::BuffShockingAura), StackingType_single, false, BoonType_Aura, new Icon(ID_Shocking_Aura, self_dll, d3d9device));
	
	tracked_buffs.emplace_back(std::vector<uint32_t>{53489}, lang.translate(LangKey::BuffSoulBarbs), StackingType_duration, false, BoonType_Necromancer, new Icon(ID_Soul_Barbs, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{10582}, lang.translate(LangKey::BuffSpectralArmor), StackingType_single, false, BoonType_Necromancer, new Icon(ID_Spectral_Armor, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{59592}, lang.translate(LangKey::BuffInspiringVirtue), StackingType_single, false, BoonType_Guardian, new Icon(ID_Inspiring_Virtue, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{44871}, lang.translate(LangKey::BuffEternalOasis), StackingType_single, false, BoonType_Guardian, new Icon(ID_Eternal_Oasis, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{43194}, lang.translate(LangKey::BuffUnbrokenLines), StackingType_single, false, BoonType_Guardian, new Icon(ID_Unbroken_Lines, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{26596, 33330}, lang.translate(LangKey::BuffRiteOfTheGreatDwarf), StackingType_single, false, BoonType_Revenant, new Icon(ID_Rite_of_the_Great_Dwarf, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{31229, 46910}, lang.translate(LangKey::BuffBulwarkGyro), StackingType_single, false, BoonType_Engineer, new Icon(ID_Bulwark_Gyro, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{56890}, lang.translate(LangKey::BuffSymbolicAvenger), StackingType_single, false, BoonType_Guardian, new Icon(ID_Symbolic_Avenger, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{30207}, lang.translate(LangKey::BuffInvigoratedBulwark), StackingType_single, false, BoonType_Guardian, new Icon(ID_Invigorated_Bulwark, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{53932}, lang.translate(LangKey::BuffStickingTogether), StackingType_single, false, BoonType_other, new Icon(ID_Sticking_Together, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{33652}, lang.translate(LangKey::BuffRigorousCertainty), StackingType_single, false, BoonType_other, new Icon(ID_Rigorous_Certainty, self_dll, d3d9device));
	
	// above 90
	above90BoonDef = std::make_shared<BoonDef>(std::vector<uint32_t>{static_cast<uint32_t>(-1)}, lang.translate(LangKey::Above90Hp), StackingType_single, false, BoonType_other, new Icon(ID_Rune_Scholar, self_dll, d3d9device)); // above 90% hp (e.g. scholar)
}

BoonDef* getTrackedBoon(uint32_t new_id) {
	for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff) {
		for (auto current_id = current_buff->ids.cbegin(); current_id != current_buff->ids.cend(); ++current_id) {
			if (*current_id == new_id) return &*current_buff;
		}
	}
	return nullptr;
}

BoonDef::BoonDef(std::vector<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category, Icon* new_icon)
: ids(new_ids), name(new_name), stacking_type(new_stacking_type), is_relevant(new_is_relevant), category(new_category), icon(new_icon) {
}

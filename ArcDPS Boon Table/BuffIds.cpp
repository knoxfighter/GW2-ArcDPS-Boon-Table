#include "BuffIds.h"

#include "Lang.h"
#include "resource.h"

std::vector<BoonDef> tracked_buffs;

void init_tracked_buffs(IDirect3DDevice9* d3d9device) {
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
	tracked_buffs.emplace_back(std::vector<uint32_t>{14222}, lang.translate(LangKey::BuffEmpowerAllies), StackingType_single, false, BoonType_trait, new Icon(ID_Empower_Allies, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{38333}, lang.translate(LangKey::BuffPinpointDistribution), StackingType_single, false, BoonType_trait, new Icon(ID_Pinpoint_Distribution, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{26854}, lang.translate(LangKey::BuffAssassinsPresence), StackingType_single, false, BoonType_trait, new Icon(ID_Assassins_Presence, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14055}, lang.translate(LangKey::BuffSpotter), StackingType_single, false, BoonType_trait, new Icon(ID_Spotter, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{5587}, lang.translate(LangKey::BuffSoothingMist), StackingType_single, false, BoonType_trait, new Icon(ID_Soothing_Mist, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{30285}, lang.translate(LangKey::BuffVampiricAura), StackingType_single, false, BoonType_trait, new Icon(ID_Vampiric_Presence, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{13796}, lang.translate(LangKey::BuffStrengthInNumbers), StackingType_single, false, BoonType_trait, new Icon(ID_Strength_in_Numbers, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14417}, lang.translate(LangKey::BuffStrength), StackingType_single, false, BoonType_banner, new Icon(ID_Banner_of_Strength2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14449}, lang.translate(LangKey::BuffBannerDiscipline), StackingType_single, false, BoonType_banner, new Icon(ID_Banner_of_Discipline2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14450}, lang.translate(LangKey::BuffBannerTactics), StackingType_single, false, BoonType_banner, new Icon(ID_Banner_of_Tactics2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{14543}, lang.translate(LangKey::BuffBannerDefense), StackingType_single, false, BoonType_banner, new Icon(ID_Banner_of_Defense2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50421}, lang.translate(LangKey::BuffSpiritFrost), StackingType_single, false, BoonType_spirit, new Icon(ID_Frost_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50413}, lang.translate(LangKey::BuffSpiritSun), StackingType_single, false, BoonType_spirit, new Icon(ID_Sun_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50415}, lang.translate(LangKey::BuffSpiritStone), StackingType_single, false, BoonType_spirit, new Icon(ID_Stone_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50381}, lang.translate(LangKey::BuffSpiritStorm), StackingType_single, false, BoonType_spirit, new Icon(ID_Storm_Spirit2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{50386}, lang.translate(LangKey::BuffSpiritWater), StackingType_single, false, BoonType_spirit, new Icon(ID_Water_Spirit, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{10235}, lang.translate(LangKey::BuffSignetInspiration), StackingType_single, false, BoonType_signet, new Icon(ID_Signet_of_Inspiration2, self_dll, d3d9device));
	tracked_buffs.emplace_back(std::vector<uint32_t>{13017, 26142, 10269 }, lang.translate(LangKey::BuffStealth), StackingType_duration, false, BoonType_other, new Icon(ID_Stealth2, self_dll, d3d9device)); //stealth + Hide in Shadows 
	tracked_buffs.emplace_back(std::vector<uint32_t>{5974}, lang.translate(LangKey::BuffSuperspeed), StackingType_single, false, BoonType_other, new Icon(ID_Super_Speed2, self_dll, d3d9device));
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

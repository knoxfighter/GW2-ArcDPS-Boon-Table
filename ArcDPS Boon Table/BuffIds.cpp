#include "BuffIds.h"

#include "Lang.h"

std::vector<BoonDef> tracked_buffs;

void init_tracked_buffs() {
	tracked_buffs.assign({
		BoonDef({740}, lang.translate(LangKey::BuffMight), StackingType_intensity, true, BoonType_boon),
		BoonDef({725}, lang.translate(LangKey::BuffFury), StackingType_duration, true, BoonType_boon),
		BoonDef({718}, lang.translate(LangKey::BuffRegeneration), StackingType_duration, false, BoonType_boon),
		BoonDef({717}, lang.translate(LangKey::BuffProtection), StackingType_duration, true, BoonType_boon),
		BoonDef({1187}, lang.translate(LangKey::BuffQuickness), StackingType_duration, true, BoonType_boon),
		BoonDef({30328}, lang.translate(LangKey::BuffAlacrity), StackingType_duration, true, BoonType_boon),
		BoonDef({873}, lang.translate(LangKey::BuffRetaliation), StackingType_duration, false, BoonType_boon),
		BoonDef({726}, lang.translate(LangKey::BuffVigor), StackingType_duration, false, BoonType_boon),
		BoonDef({1122}, lang.translate(LangKey::BuffStability), StackingType_intensity, false, BoonType_boon),
		BoonDef({743}, lang.translate(LangKey::BuffAegis), StackingType_duration, false, BoonType_boon),
		BoonDef({719}, lang.translate(LangKey::BuffSwiftness), StackingType_duration, false, BoonType_boon),
		BoonDef({26980}, lang.translate(LangKey::BuffResistance), StackingType_duration, false, BoonType_boon),
		BoonDef({14222}, lang.translate(LangKey::BuffEmpowerAllies), StackingType_single, false, BoonType_trait),
		BoonDef({38333}, lang.translate(LangKey::BuffPinpointDistribution), StackingType_single, false, BoonType_trait),
		BoonDef({26854}, lang.translate(LangKey::BuffAssassinsPresence), StackingType_single, false, BoonType_trait),
		BoonDef({14055}, lang.translate(LangKey::BuffSpotter), StackingType_single, false, BoonType_trait),
		BoonDef({5587}, lang.translate(LangKey::BuffSoothingMist), StackingType_single, false, BoonType_trait),
		BoonDef({30285}, lang.translate(LangKey::BuffVampiricAura), StackingType_single, false, BoonType_trait),
		BoonDef({13796}, lang.translate(LangKey::BuffStrengthInNumbers), StackingType_single, false, BoonType_trait),
		BoonDef({14417}, lang.translate(LangKey::BuffStrength), StackingType_single, false, BoonType_banner),
		BoonDef({14449}, lang.translate(LangKey::BuffBannerDiscipline), StackingType_single, false, BoonType_banner),
		BoonDef({14450}, lang.translate(LangKey::BuffBannerTactics), StackingType_single, false, BoonType_banner),
		BoonDef({14543}, lang.translate(LangKey::BuffBannerDefense), StackingType_single, false, BoonType_banner),
		BoonDef({50421}, lang.translate(LangKey::BuffSpiritFrost), StackingType_single, false, BoonType_spirit),
		BoonDef({50413}, lang.translate(LangKey::BuffSpiritSun), StackingType_single, false, BoonType_spirit),
		BoonDef({50415}, lang.translate(LangKey::BuffSpiritStone), StackingType_single, false, BoonType_spirit),
		BoonDef({50381}, lang.translate(LangKey::BuffSpiritStorm), StackingType_single, false, BoonType_spirit),
		BoonDef({50386}, lang.translate(LangKey::BuffSpiritWater), StackingType_single, false, BoonType_spirit),
		BoonDef({10235}, lang.translate(LangKey::BuffSignetInspiration), StackingType_single, false, BoonType_signet),
		BoonDef({13017, 26142, 10269}, lang.translate(LangKey::BuffStealth), StackingType_duration, false, BoonType_other), //stealth + Hide in Shadows
		BoonDef({5974}, lang.translate(LangKey::BuffSuperspeed), StackingType_single, false, BoonType_other),
	});
}

BoonDef* getTrackedBoon(uint32_t new_id) {
	for (auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff) {
		for (auto current_id = current_buff->ids.cbegin(); current_id != current_buff->ids.cend(); ++current_id) {
			if (*current_id == new_id) return &*current_buff;
		}
	}
	return nullptr;
}

BoonDef::BoonDef(std::initializer_list<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category) {
	ids = new_ids;
	name = new_name;
	stacking_type = new_stacking_type;
	is_relevant = new_is_relevant;
	category = new_category;
}

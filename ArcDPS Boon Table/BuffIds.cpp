#include "BuffIds.h"

std::list<BoonDef> tracked_buffs = std::list<BoonDef>({
	BoonDef(BUFF_MIGHT,"Might",StackingType_intensity,true,BoonType_boon),
	BoonDef(BUFF_FURY,"Fury",StackingType_duration,true,BoonType_boon),
	BoonDef(BUFF_REGEN,"Regen",StackingType_duration,false,BoonType_boon),
	BoonDef(BUFF_PROT,"Prot",StackingType_duration,true,BoonType_boon),
	BoonDef(BUFF_QUICK,"Quick",StackingType_duration,true,BoonType_boon),
	BoonDef(BUFF_ALAC,"Alac",StackingType_duration,true,BoonType_boon),
	BoonDef(BUFF_RETAL,"Retal",StackingType_duration,false,BoonType_boon),
	BoonDef(BUFF_VIGOR,"Vigor",StackingType_duration,false,BoonType_boon),
	BoonDef(BUFF_STAB,"Stability",StackingType_intensity,false,BoonType_boon),
	BoonDef(BUFF_AEGIS,"Aegis",StackingType_duration,false,BoonType_boon),
	BoonDef(BUFF_EA,"EA",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_PINPOINT,"PP",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_ASSASSINS_PRESENCE,"AP",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_SPOTTER,"Spotter",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_SOOTHING_MIST,"Sooth Mist",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_VAMPIRIC_AURA,"Vam Aura",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_STRENGTH_IN_NUMBERS,"Stren Num",StackingType_single,false,BoonType_trait),
	BoonDef(BUFF_BANNER_STR,"Strength",StackingType_single,false,BoonType_banner),
	BoonDef(BUFF_BANNER_DIS,"Discipline",StackingType_single,false,BoonType_banner),
	BoonDef(BUFF_BANNER_TAC,"Tactics",StackingType_single,false,BoonType_banner),
	BoonDef(BUFF_BANNER_DEF,"Defense",StackingType_single,false,BoonType_banner),
	BoonDef(BUFF_SPIRIT_FROST,"Frost",StackingType_single,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_SUN,"Sun",StackingType_single,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_STONE,"Stone",StackingType_single,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_STORM,"Storm",StackingType_single,false,BoonType_spirit),
	BoonDef(BUFF_GLYPH_EMPOW,"Empower",StackingType_single,false,BoonType_skill)
	});

bool isTrackedBoon(uint32_t new_id)
{
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.id == new_id) return true;
	}
	return false;
}

BoonDef::BoonDef(uint32_t new_id, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category)
{
	id = new_id;
	name = new_name;
	stacking_type = new_stacking_type;
	is_relevant = new_is_relevant;
	category = new_category;
}

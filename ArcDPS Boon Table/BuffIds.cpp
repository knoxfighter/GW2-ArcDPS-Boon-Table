#include "BuffIds.h"

std::list<BoonDef> tracked_buffs = std::list<BoonDef>({
	BoonDef(BUFF_MIGHT,"Might",false,true,BoonType_boon),
	BoonDef(BUFF_FURY,"Fury",true,true,BoonType_boon),
	BoonDef(BUFF_REGEN,"Regen",true,false,BoonType_boon),
	BoonDef(BUFF_PROT,"Prot",true,true,BoonType_boon),
	BoonDef(BUFF_QUICK,"Quick",true,true,BoonType_boon),
	BoonDef(BUFF_ALAC,"Alac",true,true,BoonType_boon),
	BoonDef(BUFF_RETAL,"Retal",true,false,BoonType_boon),
	BoonDef(BUFF_VIGOR,"Vigor",true,false,BoonType_boon),
	BoonDef(BUFF_STAB,"Stability",false,false,BoonType_boon),
	BoonDef(BUFF_AEGIS,"Aegis",true,false,BoonType_boon),
	BoonDef(BUFF_EA,"EA",true,false,BoonType_trait),
	BoonDef(BUFF_PINPOINT,"PP",true,false,BoonType_trait),
	BoonDef(BUFF_ASSASSINS_PRESENCE,"AP",true,false,BoonType_trait),
	BoonDef(BUFF_SPOTTER,"Spotter",true,false,BoonType_trait),
	BoonDef(BUFF_SOOTHING_MIST,"Sooth Mist",true,false,BoonType_trait),
	BoonDef(BUFF_VAMPIRIC_AURA,"Vam Aura",true,false,BoonType_trait),
	BoonDef(BUFF_STRENGTH_IN_NUMBERS,"Stren Num",true,false,BoonType_trait),
	BoonDef(BUFF_BANNER_STR,"Strength",true,false,BoonType_banner),
	BoonDef(BUFF_BANNER_DIS,"Discipline",true,false,BoonType_banner),
	BoonDef(BUFF_BANNER_TAC,"Tactics",true,false,BoonType_banner),
	BoonDef(BUFF_BANNER_DEF,"Defense",true,false,BoonType_banner),
	BoonDef(BUFF_SPIRIT_FROST,"Frost",true,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_SUN,"Sun",true,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_STONE,"Stone",true,false,BoonType_spirit),
	BoonDef(BUFF_SPIRIT_STORM,"Storm",true,false,BoonType_spirit),
	BoonDef(BUFF_GLYPH_EMPOW,"Empower",true,false,BoonType_skill)
	});

bool isTrackedBoon(uint16_t new_id)
{
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.id == new_id) return true;
	}
	return false;
}

BoonDef::BoonDef(uint16_t new_id, std::string new_name, bool new_is_duration_stacking, bool new_is_relevant, BoonType new_type)
{
	id = new_id;
	name = new_name;
	is_duration_stacking = new_is_duration_stacking;
	is_relevant = new_is_relevant;
	type = new_type;
}

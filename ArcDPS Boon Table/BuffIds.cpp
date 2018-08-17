#include "BuffIds.h"

std::list<BoonDef> tracked_buffs = std::list<BoonDef>({
	BoonDef(BUFF_MIGHT,"Might",false,true,boon),
	BoonDef(BUFF_FURY,"Fury",true,true,boon),
	BoonDef(BUFF_REGEN,"Regen",true,false,boon),
	BoonDef(BUFF_PROT,"Prot",true,true,boon),
	BoonDef(BUFF_QUICK,"Quick",true,true,boon),
	BoonDef(BUFF_ALAC,"Alac",true,true,boon),
	BoonDef(BUFF_RETAL,"Retal",true,false,boon),
	BoonDef(BUFF_VIGOR,"Vigor",true,false,boon),
	BoonDef(BUFF_STAB,"Stability",false,false,boon),
	BoonDef(BUFF_AEGIS,"Aegis",true,false,boon),
	BoonDef(BUFF_EA,"EA",true,false,buff_offensive),
	BoonDef(BUFF_PINPOINT,"PP",true,false,buff_offensive),
	BoonDef(BUFF_ASSASSINS_PRESENCE,"AP",true,false,buff_offensive),
	BoonDef(BUFF_SPOTTER,"Spot",true,false,buff_offensive),
	BoonDef(BUFF_BANNER_STR,"Strength",true,false,buff_offensive),
	BoonDef(BUFF_BANNER_DIS,"Discipline",true,false,buff_offensive),
	BoonDef(BUFF_BANNER_TAC,"Tactics",true,false,buff_defensive),
	BoonDef(BUFF_BANNER_DEF,"Defense",true,false,buff_defensive),
	BoonDef(BUFF_GLYPH_EMPOW,"Empow",true,false,buff_offensive),
	BoonDef(BUFF_SPIRIT_FROST,"Frost",true,false,buff_offensive),
	BoonDef(BUFF_SPIRIT_SUN,"Sun",true,false,buff_offensive),
	BoonDef(BUFF_SPIRIT_STONE,"Stone",true,false,buff_defensive),
	BoonDef(BUFF_SPIRIT_STORM,"Storm",true,false,buff_offensive)
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

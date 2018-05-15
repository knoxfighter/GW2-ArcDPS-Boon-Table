#include "BuffIds.h"

std::list<BoonDef> tracked_buffs = std::list<BoonDef>({
	BoonDef(BUFF_MIGHT,"Might",false,true),
	BoonDef(BUFF_FURY,"Fury",true,true),
	BoonDef(BUFF_REGEN,"Regen",true,false),
	BoonDef(BUFF_PROT,"Prot",true,true),
	BoonDef(BUFF_QUICK,"Quick",true,true),
	BoonDef(BUFF_ALAC,"Alac",true,true),
	BoonDef(BUFF_EA,"EA",true,false),
	BoonDef(BUFF_PINPOINT,"PP",true,false),
	BoonDef(BUFF_ASSASSINS_PRESENCE,"AP",true,false),
	BoonDef(BUFF_SPOTTER,"Spot",true,false),
	BoonDef(BUFF_BANNER_STR,"Strength",true,false),
	BoonDef(BUFF_BANNER_DIS,"Discipline",true,false),
	BoonDef(BUFF_BANNER_TAC,"Tactics",true,false),
	BoonDef(BUFF_BANNER_DEF,"Defense",true,false),
	BoonDef(BUFF_GLYPH_EMPOW,"Empow",true,false),
	BoonDef(BUFF_SPIRIT_FROST,"Frost",true,false),
	BoonDef(BUFF_SPIRIT_SUN,"Sun",true,false),
	BoonDef(BUFF_SPIRIT_STONE,"Stone",true,false),
	BoonDef(BUFF_SPIRIT_STORM,"Storm",true,false)
	});

bool isTrackedBoon(uint16_t new_id)
{
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.id == new_id) return true;
	}
	return false;
}

BoonDef::BoonDef(uint16_t new_id, std::string new_name, bool new_is_duration_stacking, bool new_is_relevant)
{
	id = new_id;
	name = new_name;
	is_duration_stacking = new_is_duration_stacking;
	is_relevant = new_is_relevant;
}

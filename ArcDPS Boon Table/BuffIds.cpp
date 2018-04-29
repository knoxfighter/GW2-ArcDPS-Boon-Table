#include "BuffIds.h"

std::list<BoonDef> tracked_buffs = std::list<BoonDef>({
	BoonDef(BUFF_MIGHT,"Might",false),
	BoonDef(BUFF_FURY,"Fury",true),
	BoonDef(BUFF_REGEN,"Regen",true),
	BoonDef(BUFF_PROT,"Prot",true),
	BoonDef(BUFF_QUICK,"Quick",true),
	BoonDef(BUFF_ALAC,"Alac",true),
	BoonDef(BUFF_EA,"EA",true),
	BoonDef(BUFF_PINPOINT,"PP",true),
	BoonDef(BUFF_ASSASSINS_PRESENCE,"AP",true),
	BoonDef(BUFF_SPOTTER,"SPOT",true),
	BoonDef(BUFF_BANNER_STR,"BAN STR",true),
	BoonDef(BUFF_BANNER_DIS,"BAN DIS",true),
	BoonDef(BUFF_BANNER_TAC,"BAN TAC",true),
	BoonDef(BUFF_BANNER_DEF,"BAN DEF",true),
	BoonDef(BUFF_GLYPH_EMPOW,"GLY EMP",true),
	BoonDef(BUFF_SPIRIT_FROST,"SPIR FROST",true),
	BoonDef(BUFF_SPIRIT_SUN,"SPIR SUN",true),
	BoonDef(BUFF_SPIRIT_STONE,"SPIR STON",true),
	BoonDef(BUFF_SPIRIT_STORM,"SPIR STOR",true)
	});

bool isTrackedBoon(uint16_t new_id)
{
	for (auto current_buff : tracked_buffs)
	{
		if (current_buff.id == new_id) return true;
	}
	return false;
}

BoonDef::BoonDef(uint16_t new_id, std::string new_name, bool new_is_duration_stacking)
{
	id = new_id;
	name = new_name;
	is_duration_stacking = new_is_duration_stacking;
}

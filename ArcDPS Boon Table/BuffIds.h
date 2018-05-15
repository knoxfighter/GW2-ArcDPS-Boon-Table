#pragma once

#include <inttypes.h>
#include <list>
#include <string>

const uint16_t BUFF_MIGHT = 740;
const uint16_t BUFF_FURY = 725;
const uint16_t BUFF_REGEN = 718;
const uint16_t BUFF_PROT = 717;
const uint16_t BUFF_QUICK = 1187;
const uint16_t BUFF_ALAC = 30328;
const uint16_t BUFF_EA = 14222;
const uint16_t BUFF_PINPOINT = 38333;
const uint16_t BUFF_ASSASSINS_PRESENCE = 26854;
const uint16_t BUFF_SPOTTER = 14055;
const uint16_t BUFF_BANNER_STR = 14417;
const uint16_t BUFF_BANNER_DIS = 14449;
const uint16_t BUFF_BANNER_TAC = 14450;
const uint16_t BUFF_BANNER_DEF = 14543;
const uint16_t BUFF_GLYPH_EMPOW = 31803;
const uint16_t BUFF_SPIRIT_FROST = 50421;
const uint16_t BUFF_SPIRIT_SUN = 50413;
const uint16_t BUFF_SPIRIT_STONE = 50415;
const uint16_t BUFF_SPIRIT_STORM = 50381;

bool isTrackedBoon(uint16_t new_id);
struct BoonDef
{
	uint16_t id;
	std::string name;
	bool is_duration_stacking;
	bool is_relevant;

	BoonDef(uint16_t new_id, std::string new_name, bool new_is_duration_stacking, bool new_is_relevant);
};

extern std::list<BoonDef> tracked_buffs;
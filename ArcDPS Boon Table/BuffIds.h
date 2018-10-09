#pragma once

#include <inttypes.h>
#include <list>
#include <string>

const uint32_t BUFF_MIGHT = 740;
const uint32_t BUFF_FURY = 725;
const uint32_t BUFF_REGEN = 718;
const uint32_t BUFF_PROT = 717;
const uint32_t BUFF_VIGOR = 726;
const uint32_t BUFF_QUICK = 1187;
const uint32_t BUFF_ALAC = 30328;
const uint32_t BUFF_RETAL = 873;
const uint32_t BUFF_STAB = 1122;
const uint32_t BUFF_AEGIS = 743;
const uint32_t BUFF_EA = 14222;
const uint32_t BUFF_PINPOINT = 38333;
const uint32_t BUFF_ASSASSINS_PRESENCE = 26854;
const uint32_t BUFF_SPOTTER = 14055;
const uint32_t BUFF_SOOTHING_MIST = 5587;
const uint32_t BUFF_VAMPIRIC_AURA = 30285;
const uint32_t BUFF_STRENGTH_IN_NUMBERS = 13796;
const uint32_t BUFF_BANNER_STR = 14417;
const uint32_t BUFF_BANNER_DIS = 14449;
const uint32_t BUFF_BANNER_TAC = 14450;
const uint32_t BUFF_BANNER_DEF = 14543;
const uint32_t BUFF_GLYPH_EMPOW = 31803;
const uint32_t BUFF_SPIRIT_FROST = 50421;
const uint32_t BUFF_SPIRIT_SUN = 50413;
const uint32_t BUFF_SPIRIT_STONE = 50415;
const uint32_t BUFF_SPIRIT_STORM = 50381;

bool isTrackedBoon(uint32_t new_id);

enum BoonType
{
	BoonType_boon,
	BoonType_trait,
	BoonType_banner,
	BoonType_spirit,
	BoonType_skill,
	BoonType_other,
};
struct BoonDef
{
	uint32_t id;
	std::string name;
	bool is_duration_stacking;
	bool is_relevant;
	BoonType type;

	BoonDef(uint32_t new_id, std::string new_name, bool new_is_duration_stacking, bool new_is_relevant, BoonType new_type);
};

extern std::list<BoonDef> tracked_buffs;
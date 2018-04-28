#include "BuffIds.h"

auto tracked_buffs = std::list<uint16_t>({
	BUFF_MIGHT,
	BUFF_FURY,
	BUFF_REGEN,
	BUFF_PROT,
	BUFF_QUICK,
	BUFF_ALAC,
	BUFF_EA,
	BUFF_PINPOINT,
	BUFF_ASSASSINS_PRESENCE,
	BUFF_SPOTTER,
	BUFF_BANNER_STR,
	BUFF_BANNER_DIS,
	BUFF_BANNER_TAC,
	BUFF_BANNER_DEF,
	BUFF_GLYPH_EMPOW,
	BUFF_SPIRIT_FROST,
	BUFF_SPIRIT_SUN,
	BUFF_SPIRIT_STONE,
	BUFF_SPIRIT_STORM
	});

bool isTrackedBoon(uint16_t new_id)
{
	for (auto current_id : tracked_buffs)
	{
		if (current_id == new_id) return true;
	}
	return false;
}

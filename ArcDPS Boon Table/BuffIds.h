#pragma once

#include <inttypes.h>
#include <list>
#include <string>

enum BoonType
{
	BoonType_boon,
	BoonType_trait,
	BoonType_banner,
	BoonType_spirit,
	BoonType_skill,
	BoonType_other,
};

enum StackingType
{
	StackingType_duration,
	StackingType_intensity,
	StackingType_single
};

struct BoonDef
{
	uint32_t id = 0;
	std::string name = "";
	StackingType stacking_type = StackingType_duration;
	bool is_relevant = false;
	BoonType category = BoonType_other;

	BoonDef(uint32_t new_id, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category);
};

BoonDef* getTrackedBoon(uint32_t new_id);

extern std::list<BoonDef> tracked_buffs;
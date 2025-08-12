#pragma once

#include <string>
#include <vector>
#include <memory>

#include "extension/Icon.h"

enum BoonType
{
	BoonType_boon,
	BoonType_Guardian,
	BoonType_Ranger,
	BoonType_Warrior,
	BoonType_Revenant,
	BoonType_Engineer,
	BoonType_Elementalist,
	BoonType_Necromancer,
	BoonType_Mesmer,
	BoonType_Aura,
	BoonType_other,
	BoonType_Relic,
	BoonType_None,
};

enum StackingType
{
	StackingType_duration,
	StackingType_intensity,
	StackingType_single
};

struct BoonDef
{
	std::vector<uint32_t> ids = {};
	std::string name;
	StackingType stacking_type = StackingType_duration;
	bool is_relevant = false;
	BoonType category = BoonType_None;
	UINT icon = 0;

	BoonDef(std::string new_name) : name(std::move(new_name)) {}
	BoonDef(std::vector<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category, UINT new_icon);
};

BoonDef* getTrackedBoon(uint32_t new_id);

extern std::vector<BoonDef> tracked_buffs;
extern std::shared_ptr<BoonDef> above90BoonDef;

void init_tracked_buffs();

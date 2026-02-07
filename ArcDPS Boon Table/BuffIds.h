#pragma once

#include <string>
#include <vector>
#include <memory>

#include <ArcdpsExtension/IconLoader.h>

#include "Lang.h"

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
	LangKey name;
	StackingType stacking_type = StackingType_duration;
	bool is_relevant = false;
	BoonType category = BoonType_None;
	UINT icon = 0;
	size_t iconTextureId = 0;
	uint8_t max_stacks = 25; // Only used if StackingType == StackingType_intensity

	template<typename T>
	requires std::is_enum_v<T> or std::is_integral_v<T>
	BoonDef(T new_name) : name(static_cast<LangKey>(new_name)) {}

	template<typename T>
	requires std::is_enum_v<T> or std::is_integral_v<T>
	BoonDef(std::vector<uint32_t> new_ids, T new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category, UINT new_icon, uint32_t new_max_stacks = 25)
		: ids(new_ids), name(static_cast<LangKey>(new_name)), stacking_type(new_stacking_type), is_relevant(new_is_relevant), category(new_category), icon(new_icon), max_stacks(new_max_stacks) {
		
	}

	bool IsValid() const
	{
		return !ids.empty();
	}
};

BoonDef* getTrackedBoon(uint32_t new_id);

extern std::vector<BoonDef> tracked_buffs;
extern std::shared_ptr<BoonDef> above90BoonDef;

void init_tracked_buffs();

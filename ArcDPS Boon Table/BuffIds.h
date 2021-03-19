#pragma once

#include <string>
#include <vector>
#include <d3d9.h>

#include "extension/Icon.h"

enum BoonType
{
	BoonType_boon,
	BoonType_trait,
	BoonType_banner,
	BoonType_spirit,
	BoonType_skill,
	BoonType_signet,
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
	std::vector<uint32_t> ids = {};
	std::string name = "";
	StackingType stacking_type = StackingType_duration;
	bool is_relevant = false;
	BoonType category = BoonType_other;
	Icon* icon;

	BoonDef(std::vector<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category, Icon* new_icon);
};

BoonDef* getTrackedBoon(uint32_t new_id);

extern std::vector<BoonDef> tracked_buffs;
void init_tracked_buffs(IDirect3DDevice9* d3d9device);
extern HMODULE self_dll;
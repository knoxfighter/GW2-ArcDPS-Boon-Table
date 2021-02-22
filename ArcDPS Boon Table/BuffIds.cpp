#include "BuffIds.h"

std::vector<BoonDef> tracked_buffs = std::vector<BoonDef>({
	BoonDef({740},"Might",StackingType_intensity,true,BoonType_boon),
	BoonDef({725},"Fury",StackingType_duration,true,BoonType_boon),
	BoonDef({718},"Regen",StackingType_duration,false,BoonType_boon),
	BoonDef({717},"Prot",StackingType_duration,true,BoonType_boon),
	BoonDef({1187},"Quick",StackingType_duration,true,BoonType_boon),
	BoonDef({30328},"Alac",StackingType_duration,true,BoonType_boon),
	BoonDef({873},"Retal",StackingType_duration,false,BoonType_boon),
	BoonDef({726},"Vigor",StackingType_duration,false,BoonType_boon),
	BoonDef({1122},"Stability",StackingType_intensity,false,BoonType_boon),
	BoonDef({743},"Aegis",StackingType_duration,false,BoonType_boon),
	BoonDef({719},"Swiftness",StackingType_duration,false,BoonType_boon),
	BoonDef({26980},"Resistance",StackingType_duration,false,BoonType_boon),
	BoonDef({14222},"EA",StackingType_single,false,BoonType_trait),
	BoonDef({38333},"PP",StackingType_single,false,BoonType_trait),
	BoonDef({26854},"AP",StackingType_single,false,BoonType_trait),
	BoonDef({14055},"Spotter",StackingType_single,false,BoonType_trait),
	BoonDef({5587},"Sooth Mist",StackingType_single,false,BoonType_trait),
	BoonDef({30285},"Vam Aura",StackingType_single,false,BoonType_trait),
	BoonDef({13796},"Stren Num",StackingType_single,false,BoonType_trait),
	BoonDef({14417},"Strength",StackingType_single,false,BoonType_banner),
	BoonDef({14449},"Discipline",StackingType_single,false,BoonType_banner),
	BoonDef({14450},"Tactics",StackingType_single,false,BoonType_banner),
	BoonDef({14543},"Defense",StackingType_single,false,BoonType_banner),
	BoonDef({50421},"Frost",StackingType_single,false,BoonType_spirit),
	BoonDef({50413},"Sun",StackingType_single,false,BoonType_spirit),
	BoonDef({50415},"Stone",StackingType_single,false,BoonType_spirit),
	BoonDef({50381},"Storm",StackingType_single,false,BoonType_spirit),
	BoonDef({50386},"Water",StackingType_single,false,BoonType_spirit),
	BoonDef({10235},"Inspiration",StackingType_single,false,BoonType_signet),
	BoonDef({13017,26142,10269},"Stealth",StackingType_duration,false,BoonType_other),//stealth + Hide in Shadows
	BoonDef({5974},"Superspeed",StackingType_single,false,BoonType_other),
	});

BoonDef* getTrackedBoon(uint32_t new_id)
{
	for(auto current_buff = tracked_buffs.begin(); current_buff != tracked_buffs.end(); ++current_buff)
	{
		for (auto current_id = current_buff->ids.cbegin(); current_id != current_buff->ids.cend(); ++current_id)
		{
			if (*current_id == new_id) return &*current_buff;
		}
	}
	return nullptr;
}

BoonDef::BoonDef(std::initializer_list<uint32_t> new_ids, std::string new_name, StackingType new_stacking_type, bool new_is_relevant, BoonType new_category)
{
	ids = new_ids;
	name = new_name;
	stacking_type = new_stacking_type;
	is_relevant = new_is_relevant;
	category = new_category;
}

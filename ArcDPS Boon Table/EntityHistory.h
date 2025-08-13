#pragma once

#include "Entity.h"
#include "IEntity.h"

#include <ranges>

class EntityHistory : public virtual IEntity {
protected:
	uintptr_t id;
	std::string name;
	uint64_t combatDuration = 0;
	float over90 = 0;
	std::map<uint32_t, float> boonUptime;
	
public:
	EntityHistory() = default;
	EntityHistory(const Entity& entity) {
		id = entity.getId();
		name = entity.getName();
		over90 = entity.getOver90();
		combatDuration = entity.getCombatDuration();
		for (const BoonDef& trackedBuff : tracked_buffs | std::views::filter(&BoonDef::IsValid)) {
			boonUptime[trackedBuff.ids[0]] = entity.getBoonUptime(trackedBuff);
		}
	}

	bool operator==(std::string other_name) const;
	
	uintptr_t getId() const override;
	const std::string& getName() const override;
	float getOver90() const override;
	ImVec4 getColor() const override;
	uint64_t getCombatDuration() const override;
	float getBoonUptime(const BoonDef& boon) const override;
};

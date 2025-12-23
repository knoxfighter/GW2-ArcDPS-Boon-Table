#pragma once

#include <string>
#include <map>
#include <mutex>

#include <ArcdpsExtension/arcdps_structs.h>
#include "Helpers.h"
#include "Boon.h"
#include "BuffIds.h"
#include "IEntity.h"

class Entity : public virtual IEntity {
protected:
	uintptr_t id;
	std::string name;

	mutable std::mutex boons_mtx;

	std::map<uint32_t, Boon> boons_uptime;
	std::map<uint32_t, Boon> boons_uptime_initial;
	std::map<uint32_t, Boon> boons_generation;
	std::map<uint32_t, Boon> boons_generation_initial;

public:
	virtual bool operator==(uintptr_t other_id) const;
	virtual bool operator==(std::string other_name) const;

	Entity() = default;
	Entity(const Entity& other)
		: id(other.id),
		  name(other.name),
		  boons_uptime(other.boons_uptime),
		  boons_uptime_initial(other.boons_uptime_initial),
		  boons_generation(other.boons_generation),
		  boons_generation_initial(other.boons_generation_initial),
		  enter_combat_time(other.enter_combat_time),
		  exit_combat_time(other.exit_combat_time),
		  in_combat(other.in_combat) {
	}

	Entity& operator=(const Entity& other) {
		if (this == &other)
			return *this;
		id = other.id;
		name = other.name;
		boons_uptime = other.boons_uptime;
		boons_uptime_initial = other.boons_uptime_initial;
		boons_generation = other.boons_generation;
		boons_generation_initial = other.boons_generation_initial;
		enter_combat_time = other.enter_combat_time;
		exit_combat_time = other.exit_combat_time;
		in_combat = other.in_combat;
		return *this;
	}

	virtual bool operator==(const Entity& other) const;

	uintptr_t getId() const override;
	const std::string& getName() const override;
	
	void applyBoon(cbtevent* ev);
	void removeBoon(cbtevent* ev);
	void gaveBoon(cbtevent* ev);
	void flushAllBoons();
	void dealtDamage(cbtevent* ev);

	float getBoonUptime(const BoonDef& boon) const override;
	float getBoonGeneration(const BoonDef& new_boon) const;

	virtual void combatEnter(cbtevent* ev);
	void combatExit(cbtevent* ev);
	uint64_t getCombatDuration() const override;
	uint64_t enter_combat_time = getCurrentTime();
	uint64_t exit_combat_time = getCurrentTime();
	bool in_combat = false;

	std::atomic_uint32_t damageEvents = 0;
	std::atomic_uint32_t damageEventsOver90 = 0;
	float getOver90() const override;

	ImVec4 getColor() const override;
};

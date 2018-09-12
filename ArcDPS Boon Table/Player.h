#pragma once

#include <string>
#include <list>
#include <mutex>
#include "ArcdpsDataStructures.h"
#include "Helpers.h"
#include "Boon.h"
#include "BuffIds.h"

class Player
{
public:
	uintptr_t id;
	std::string name;
	std::string account_name;
	std::list<Boon> boons;
	uint64_t enter_combat_time;
	uint64_t exit_combat_time;
	bool in_combat;
	uint8_t subgroup;
	bool is_relevant;

	bool operator==(uintptr_t other_id);

	Player();
	Player(uintptr_t new_id, std::string new_name, std::string new_account_name);
	~Player();

	bool isRelevant();

	void applyBoon(cbtevent* ev);
	void removeBoon(cbtevent* ev);
	
	float getBoonUptime(BoonDef* new_boon);
	bool hasBoonNow(BoonDef* new_boon);

	void combatEnter(uint64_t new_time, uint8_t new_subgroup);
	void combatExit(uint64_t new_time);
	float getCombatTime();
};

extern std::mutex boons_mtx;
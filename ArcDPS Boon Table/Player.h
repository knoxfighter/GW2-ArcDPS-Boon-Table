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
	uintptr_t id = 0;
	std::string name = "";
	std::string account_name = "";
	std::list<Boon> boons;
	uint64_t enter_combat_time = getCurrentTime();
	uint64_t exit_combat_time = getCurrentTime();;
	bool in_combat = false;
	uint8_t subgroup = 0;
	bool is_relevant = true;

	bool operator==(uintptr_t other_id);
	bool operator==(std::string other_name);

	Player(uintptr_t new_id, std::string new_name, std::string new_account_name, uint8_t new_subgroup);
	~Player();

	bool isRelevant();

	void applyBoon(cbtevent* ev);
	void removeBoon(cbtevent* ev);

	Boon* getBoon(uint32_t new_boon);
	
	float getBoonUptime(BoonDef* new_boon);
	bool hasBoonNow(BoonDef* new_boon);

	void combatEnter(uint64_t new_time, uint8_t new_subgroup);
	void combatExit(uint64_t new_time);
	uint64_t getCombatTime();
};

extern std::mutex boons_mtx;
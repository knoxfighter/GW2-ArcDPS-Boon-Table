#pragma once

#include <string>
#include <map>
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
	prof profession = PROF_UNKNOWN;
	std::map<uint32_t, Boon> boons_uptime;
	std::map<uint32_t, Boon> boons_uptime_initial;
	std::map<uint32_t, Boon> boons_generation;
	std::map<uint32_t, Boon> boons_generation_initial;
	uint64_t enter_combat_time = getCurrentTime();
	uint64_t exit_combat_time = getCurrentTime();;
	bool in_combat = false;
	uint8_t subgroup = 1;

	bool operator==(uintptr_t other_id) const;
	bool operator==(std::string other_name) const;
	bool operator==(const Player& other) const;

	Player(uintptr_t new_id, const std::string& new_name, const std::string& new_account_name, uint8_t new_subgroup, prof new_profession);

	void applyBoon(cbtevent* ev);
	void removeBoon(cbtevent* ev);
	void gaveBoon(cbtevent* ev);
	void flushAllBoons();

	float getBoonUptime(const BoonDef& boon) const;
	float getBoonGeneration(const BoonDef& new_boon) const;

	void combatEnter(cbtevent* ev);
	void combatExit(cbtevent* ev);
	uint64_t getCombatDuration() const;

	ImVec4 getProfessionColor() const;
};

extern std::mutex boons_mtx;
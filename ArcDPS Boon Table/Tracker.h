#pragma once
#include <list>
#include <string>
#include <algorithm>
#include <mutex>

#include "ArcdpsDataStructures.h"
#include "Player.h"

class Tracker
{
public:
	std::mutex players_mtx;
	std::list<Player> players;
	std::list<uint8_t> subgroups;

	Tracker();
	~Tracker();

	bool addPlayer(uintptr_t new_id, std::string name);
	bool removePlayer(uintptr_t new_id);

	void bakeCombatData();

	Player* getPlayer(ag* new_player);
	
	std::list<uint8_t> getSubgroups();
	float getSubgroupBoonUptime(BoonDef new_boon, uint8_t new_subgroup);
	float getAverageBoonUptime(BoonDef new_boon);
};


#pragma once
#include <list>
#include <string>
#include <algorithm>
#include <mutex>

#include "ArcdpsDataStructures.h"
#include "Player.h"

enum SortMethod
{
	SortMethod_name,
	SortMethod_subgroup,
	SortMethod_boon
};

class Tracker
{
protected:
	SortMethod sort_method;
	BoonDef* sorted_boon;
	bool sort_reverse;

	bool needs_resort;
public:
	std::mutex players_mtx;
	std::list<Player> players;

	std::mutex subgroups_mtx;
	std::list<uint8_t> subgroups;

	Tracker();
	~Tracker();

	bool addPlayer(uintptr_t new_id, std::string name);
	bool removePlayer(uintptr_t new_id, std::string new_name);
	void clearPlayers();//marks all players as not in squad
	

	void sortPlayers();
	void setSortMethod(SortMethod new_method, BoonDef* new_boon = nullptr);
	void queueResort();

	void bakeCombatData();

	Player* getPlayer(ag* new_player);

	uint16_t getRelevantPlayerCount();
	
	std::list<uint8_t> getSubgroups();
	float getSubgroupBoonUptime(BoonDef* new_boon, uint8_t new_subgroup);
	float getAverageBoonUptime(BoonDef* new_boon);
};


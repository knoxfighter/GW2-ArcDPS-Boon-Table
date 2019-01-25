#pragma once
#include <list>
#include <string>
#include <algorithm>
#include <mutex>

#include "ArcdpsDataStructures.h"
#include "imgui/imgui.h"
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
	float max_character_name_size = 75.0f;
	bool is_squad = false;
	int relevant_player_count = 0;

	std::mutex players_mtx;
	std::list<Player> players;

	std::mutex subgroups_mtx;
	std::list<uint8_t> subgroups;

	Tracker();
	~Tracker();

	void addPlayer(ag* src, ag* dst);
	void removePlayer(ag* src);
	void clearPlayers();//marks all players as not in squad
	

	void sortPlayers();
	void setSortMethod(SortMethod new_method, BoonDef* new_boon = nullptr);
	void queueResort();

	void bakeCombatData();

	Player* getPlayer(uintptr_t new_player);
	Player* getPlayer(std::string new_player);
	
	std::list<uint8_t> getSubgroups();
	float getSubgroupBoonUptime(BoonDef* new_boon, uint8_t new_subgroup);
	float getAverageBoonUptime(BoonDef* new_boon);
};


#pragma once
#include <list>
#include <string>
#include <mutex>

#include "ArcdpsDataStructures.h"
#include "Player.h"

class Tracker
{
public:
	bool is_squad = false;

	mutable std::mutex players_mtx;
	std::list<Player> players;
	//mutable std::mutex npcs_mtx;
	//std::list<NPC> npcs;

	std::mutex subgroups_mtx;
	std::list<uint8_t> subgroups;

	Tracker() = default;

	void addPlayer(ag* src, ag* dst);
	void addPlayer(uintptr_t id, uint8_t subgroup, prof profession, std::string characterName, std::string accountName);
	void addNewPlayer(uintptr_t id, uint8_t subgroup, prof profession, std::string characterName, std::string accountName);
	void removePlayer(ag* src);
	void removeEntity(Entity* entity);
	void removeEntity(Player* player);
	void clearPlayers();//marks all players as not in squad
	void bakeCombatData();

	Player* getPlayer(uintptr_t new_player);
	Player* getPlayer(std::string new_player);

	void applyBoon(ag* src, ag* dst, cbtevent* ev);
	
	std::list<uint8_t> getSubgroups();
	float getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const;
	float getAverageBoonUptime(const BoonDef& boon) const;
};


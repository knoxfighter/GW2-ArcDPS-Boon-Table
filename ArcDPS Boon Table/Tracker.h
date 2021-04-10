#pragma once

#include <list>
#include <string>
#include <mutex>
#include <set>
#include <unordered_map>


#include "extension/arcdps_structs.h"
#include "Player.h"
#include "NPC.h"

class Tracker
{
public:
	bool is_squad = false;

	mutable std::mutex players_mtx;
	std::unordered_map<uintptr_t, Player> players;
	mutable std::mutex npcs_mtx;
	std::list<NPC> npcs;

	std::mutex subgroups_mtx;

	std::set<uint8_t> subgroups;

	Tracker() = default;

	void addPlayer(ag* src, ag* dst);
	void addPlayer(uintptr_t id, uint8_t subgroup, Prof profession, std::string characterName, std::string accountName);
	void addNewPlayer(uintptr_t id, uint8_t subgroup, Prof profession, std::string characterName, std::string accountName);
	void addNPC(uintptr_t id, std::string name, cbtevent* ev);
	void removePlayer(ag* src);
	void clearPlayers();//marks all players as not in squad
	void clearNPCs();//marks all npcs as not in squad
	void bakeCombatData();

	Player* getPlayer(uintptr_t new_player);
	Player* getPlayer(std::string new_player);

	NPC* getNPC(uintptr_t new_npc);
	NPC* getNPC(std::string new_name);

	Entity* getEntity(uintptr_t new_npc);

	void applyBoon(ag* src, ag* dst, cbtevent* ev);
	void dealtDamage(ag* src, cbtevent* ev);
	
	std::set<uint8_t> getSubgroups();
	float getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const;
	float getSubgroupOver90(uint8_t subgroup) const;
	
	float getAverageBoonUptime(const BoonDef& boon) const;
	float getAverageOver90() const;
};


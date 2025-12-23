#pragma once

#include <list>
#include <string>
#include <mutex>
#include <set>
#include <unordered_map>

#include "ITracker.h"
#include <ArcdpsExtension/arcdps_structs.h>
#include "Player.h"
#include "NPC.h"

class Tracker : public ITracker {
protected:
	std::unordered_map<uintptr_t, Player> players;

public:
	Tracker() = default;

	bool isSquad() const override;
	
	void addPlayer(ag* src, ag* dst);
	void addPlayer(uintptr_t id, uint8_t subgroup, Prof profession, std::string characterName, std::string accountName, bool self);
	void addNewPlayer(uintptr_t id, uint8_t subgroup, Prof profession, std::string characterName, std::string accountName, bool self);
	void removePlayer(ag* src);
	void clearPlayers();//marks all players as not in squad

	Player* getPlayer(uintptr_t new_player);
	Player* getPlayer(std::string new_player);
	IPlayer* getIPlayer(uintptr_t new_player) override;
	IPlayer* getIPlayer(std::string new_player) override;
	IPlayer* getSelfIPlayer() override;
	std::unordered_map<uintptr_t, Player>& getPlayers();
	std::set<uintptr_t> getAllPlayerIds() override;

	Entity* getEntity(uintptr_t new_entity);
	IEntity* getIEntity(uintptr_t new_entity) override;
	
	void applyBoon(ag* src, ag* dst, cbtevent* ev);
	void dealtDamage(ag* src, cbtevent* ev);
	
	std::set<uint8_t> getSubgroups() const override;
	float getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const override;
	float getSubgroupOver90(uint8_t subgroup) const override;
	
	float getAverageBoonUptime(const BoonDef& boon) const override;
	float getAverageOver90() const override;
};

extern Tracker liveTracker;

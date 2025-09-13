#pragma once

#include <set>

#include "BuffIds.h"
#include "IPlayer.h"

class ITracker {
public:
	mutable std::mutex players_mtx;
	
	virtual std::set<uint8_t> getSubgroups() const = 0;
	virtual float getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const = 0;
	virtual float getSubgroupOver90(uint8_t subgroup) const = 0;
	
	virtual float getAverageBoonUptime(const BoonDef& boon) const = 0;
	virtual float getAverageOver90() const = 0;

	virtual IPlayer* getIPlayer(uintptr_t new_player) = 0;
	virtual IPlayer* getIPlayer(std::string new_player) = 0;
	virtual IPlayer* getSelfIPlayer() = 0;
	virtual std::set<uintptr_t> getAllPlayerIds() = 0;

	virtual IEntity* getIEntity(uintptr_t new_entity) = 0;

	virtual bool isSquad() const = 0;
};

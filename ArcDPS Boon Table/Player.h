#pragma once

#include <string>
#include <mutex>

#include "extension/arcdps_structs.h"
#include "Boon.h"
#include "Entity.h"
#include "IPlayer.h"

class Tracker;

class Player : public Entity, IPlayer {
	friend Tracker;
	
protected:
	std::string account_name;
	Prof profession;
	uint8_t subgroup;
	bool self = false;

public:
	Player(uintptr_t new_id, const std::string& new_name, const std::string& new_account_name, uint8_t new_subgroup, Prof new_profession, bool new_self);

	[[nodiscard]] uint8_t getSubgroup() const override;
	[[nodiscard]] bool isSelf() const override;
	[[nodiscard]] Prof getProfession() const override;
	
	void combatEnter(cbtevent* ev) override;
	ImVec4 getColor() const override;

	bool operator==(uintptr_t other_id) const override;
	bool operator==(std::string other_name) const override;
	bool operator==(const Entity& other) const override;
};

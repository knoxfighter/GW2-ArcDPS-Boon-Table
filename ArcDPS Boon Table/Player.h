#pragma once

#include <string>
#include <mutex>

#include "extension/arcdps_structs.h"
#include "Helpers.h"
#include "Boon.h"
#include "Entity.h"

class Player : public Entity
{
public:
	std::string account_name;
	Prof profession;
	uint8_t subgroup;
	bool self = false;

	Player(uintptr_t new_id, const std::string& new_name, const std::string& new_account_name, uint8_t new_subgroup, Prof new_profession);

	void combatEnter(cbtevent* ev) override;
	ImVec4 getColor() const override;

	bool operator==(uintptr_t other_id) const override;
	bool operator==(std::string other_name) const override;
	bool operator==(const Entity& other) const override;
};
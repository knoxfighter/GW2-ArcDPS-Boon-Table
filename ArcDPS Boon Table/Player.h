#pragma once

#include <string>
#include <map>
#include <mutex>
#include "ArcdpsDataStructures.h"
#include "Helpers.h"
#include "Boon.h"
#include "BuffIds.h"
#include "Entity.h"

class Player : public Entity
{
public:
	std::string account_name;
	prof profession;
	uint8_t subgroup;

	Player(uintptr_t new_id, const std::string& new_name, const std::string& new_account_name, uint8_t new_subgroup, prof new_profession);

	bool operator==(std::string other_name) const;

	void combatEnter(cbtevent* ev);
	ImVec4 getColor() const;
};
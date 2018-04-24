#pragma once

#include <string>
#include <list>
#include "Boon.h"

class Player
{
public:
	uintptr_t id;
	std::string name;
	std::list<Boon> boons;

	bool operator==(uintptr_t other_id);

	Player();
	Player(uintptr_t new_id, std::string new_name);
	~Player();
};
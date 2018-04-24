#pragma once
#include <list>
#include <string>
#include <algorithm>

#include "Player.h"

class Tracker
{
public:
	std::list<Player> players;

	Tracker();
	~Tracker();

	Player addPlayer(uintptr_t new_id, std::string name);
	bool removePlayer(uintptr_t new_id);
};


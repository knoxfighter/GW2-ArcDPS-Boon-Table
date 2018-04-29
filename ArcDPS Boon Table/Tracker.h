#pragma once
#include <list>
#include <string>
#include <algorithm>
#include <mutex>

#include "ArcdpsDataStructures.h"
#include "Player.h"

class Tracker
{
public:
	std::mutex players_mtx;
	std::list<Player> players;

	Tracker();
	~Tracker();

	bool addPlayer(uintptr_t new_id, std::string name);
	bool removePlayer(uintptr_t new_id);

	Player* getPlayer(ag* new_player);
};


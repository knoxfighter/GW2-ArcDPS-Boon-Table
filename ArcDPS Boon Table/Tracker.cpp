#include "Tracker.h"



Tracker::Tracker()
{
}


Tracker::~Tracker()
{
}

bool Tracker::addPlayer(uintptr_t new_id, std::string new_name)
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find(players.begin(), players.end(), new_id);

	//player not tracked yet
	if (it == players.end())
	{
		players.push_back(Player(new_id, new_name));
		return true;
	}
	else//player tracked
	{
		return false;
	}
}

bool Tracker::removePlayer(uintptr_t new_id)
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find(players.begin(), players.end(), new_id);

	//player not tracked yet
	if (it == players.end())
	{
		return false;
	}
	else//player tracked
	{
		players.erase(it);
		return true;
	}
}

Player* Tracker::getPlayer(ag* new_player)
{
	if (!is_player(new_player)) return nullptr;
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find(players.begin(), players.end(), new_player->id);

	//player not tracked yet
	if (it == players.end())
	{
		return nullptr;
	}
	else//player tracked
	{
		return &*it;
	}
}

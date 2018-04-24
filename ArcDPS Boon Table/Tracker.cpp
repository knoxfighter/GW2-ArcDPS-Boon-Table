#include "Tracker.h"



Tracker::Tracker()
{
}


Tracker::~Tracker()
{
}

Player Tracker::addPlayer(uintptr_t new_id, std::string new_name)
{
	auto it = std::find(players.begin(), players.end(), new_id);

	//player not tracked yet
	if (it == players.end())
	{
		players.push_back(Player(new_id, new_name));
		return players.back();
	}
	else//player tracked
	{
		return *it;
	}
}

bool Tracker::removePlayer(uintptr_t new_id)
{
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

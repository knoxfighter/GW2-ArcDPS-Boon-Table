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

std::list<uint8_t> Tracker::getSubgroups()
{
	auto out = std::list<uint8_t>();
	bool found = false;

	for (auto player : players)
	{
		for (auto current_sub : out)
		{
			if (player.subgroup == current_sub)
			{
				found = true;
				break;
			}
		}
		if (found)
		{
			found = false;
			continue;
		}
		else
		{
			out.push_back(player.subgroup);
		}
	}
	out.sort();
	return out;
}

float Tracker::getSubgroupBoonUptime(uint16_t new_boon_id, uint8_t new_subgroup)
{
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto player : players)
	{
		if (player.subgroup != new_subgroup) continue;

		out += player.getBoonUptime(new_boon_id);
		player_num++;
	}
	if (player_num == 0) return out;
	else return out / player_num;
}

float Tracker::getAverageBoonUptime(uint16_t new_boon_id)
{
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto player : players)
	{
		out += player.getBoonUptime(new_boon_id);
		player_num++;
	}
	if (player_num == 0) return out;
	else return out / player_num;
}

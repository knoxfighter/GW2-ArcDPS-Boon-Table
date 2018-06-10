#include "Tracker.h"



Tracker::Tracker()
{
	sort_method = subgroup;
	sorted_boon = nullptr;
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

void Tracker::sortPlayers()
{
	switch (sort_method)
	{
		case name:
		{
			players.sort();
			break;
		}
		case subgroup:
		{
			players.sort([](Player lhs, Player rhs) {return lhs.subgroup < rhs.subgroup; });
			break;
		}
		case boon:
		{
			players.sort([this](Player lhs, Player rhs) {return lhs.getBoonUptime(sorted_boon) < rhs.getBoonUptime(sorted_boon); });
			break;
		}
		break;
	};

}

void Tracker::setSortMethod(SortMethod new_method, BoonDef * new_boon)
{
	if (new_method == boon && !new_boon) return;
	sort_method = new_method;
	if (new_boon) sorted_boon = new_boon;
}

void Tracker::bakeCombatData()
{
	subgroups = getSubgroups();
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
	std::lock_guard<std::mutex> lock(subgroups_mtx);
	std::lock_guard<std::mutex> lock2(players_mtx);
	auto out = std::list<uint8_t>();
	bool found = false;

	for (std::list<Player>::iterator player = players.begin(); player != players.end(); ++player)
	{
		for (auto current_sub : out)
		{
			if (player->subgroup == current_sub)
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
			out.push_back(player->subgroup);
		}
	}
	out.sort();
	return out;
}

float Tracker::getSubgroupBoonUptime(BoonDef* new_boon, uint8_t new_subgroup)
{
	float out = 0.0f;
	uint8_t player_num = 0;

	for (std::list<Player>::iterator player = players.begin(); player != players.end(); ++player)
	{
		if (player->subgroup != new_subgroup) continue;

		out += player->getBoonUptime(new_boon);
		player_num++;
	}
	if (player_num == 0) return out;
	else return out / player_num;
}

float Tracker::getAverageBoonUptime(BoonDef* new_boon)
{
	float out = 0.0f;
	uint8_t player_num = 0;

	for (std::list<Player>::iterator player = players.begin(); player != players.end(); ++player)
	{
		out += player->getBoonUptime(new_boon);
		player_num++;
	}
	if (player_num == 0) return out;
	else return out / player_num;
}

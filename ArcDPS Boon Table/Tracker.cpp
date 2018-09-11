#include "Tracker.h"



Tracker::Tracker()
{
	sort_method = SortMethod_subgroup;
	sorted_boon = nullptr;
	sort_reverse = false;
	needs_resort = true;
}


Tracker::~Tracker()
{
}

bool Tracker::addPlayer(uintptr_t new_id, std::string new_name)
{
	std::unique_lock<std::mutex> lock(players_mtx);

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (player->id == new_id || player->name == new_name)
		{
			player->is_relevant = true;
			player->id = new_id;
			return false;
		}
	}

	players.push_back(Player(new_id, new_name));
	lock.unlock();
	bakeCombatData();
	return true;	
}

bool Tracker::removePlayer(uintptr_t new_id, std::string new_name)
{
	std::unique_lock<std::mutex> lock(players_mtx);

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (player->id == new_id || player->name == new_name)
		{
			player->is_relevant = false;
			lock.unlock();
			bakeCombatData();
			return true;
		}
	}

	return false;
}

void Tracker::clearPlayers()
{
	std::unique_lock<std::mutex> lock(players_mtx);

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		player->is_relevant = false;
	}
	lock.unlock();
	bakeCombatData();
}

void Tracker::sortPlayers()
{
	if (!needs_resort) return;
	std::lock_guard<std::mutex> lock(players_mtx);
	switch (sort_method)
	{
		case SortMethod_name:
		{
			players.sort([this](Player lhs, Player rhs) {return sort_reverse ? lhs.name > rhs.name : lhs.name < rhs.name; });
			break;
		}
		case SortMethod_subgroup:
		{
			players.sort([this](Player lhs, Player rhs) {return sort_reverse ? lhs.subgroup > rhs.subgroup : lhs.subgroup < rhs.subgroup; });
			break;
		}
		case SortMethod_boon:
		{
			players.sort([this](Player lhs, Player rhs) {return sort_reverse ? lhs.getBoonUptime(sorted_boon) > rhs.getBoonUptime(sorted_boon) : lhs.getBoonUptime(sorted_boon) < rhs.getBoonUptime(sorted_boon); });
			break;
		}
		break;
	};
	needs_resort = false;
}

void Tracker::setSortMethod(SortMethod new_method, BoonDef * new_boon)
{
	if (new_method == SortMethod_boon && !new_boon) return;
	if (sort_method == new_method) sort_reverse = !sort_reverse;
	sort_method = new_method;
	if (new_boon) sorted_boon = new_boon;
	queueResort();
}

void Tracker::queueResort()
{
	needs_resort = true;
}

void Tracker::bakeCombatData()
{
	std::lock_guard<std::mutex> lock(subgroups_mtx);
	subgroups = getSubgroups();
	queueResort();
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

uint16_t Tracker::getRelevantPlayerCount()
{
	uint16_t out = 0;

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (player->isRelevant()) out++;
	}
	
	return out;
}

std::list<uint8_t> Tracker::getSubgroups()
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto out = std::list<uint8_t>();
	bool found = false;

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (!player->isRelevant()) continue;
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

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (!player->isRelevant()) continue;
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

	for (auto player = players.begin(); player != players.end(); ++player)
	{
		if (!player->isRelevant()) continue;
		out += player->getBoonUptime(new_boon);
		player_num++;
	}
	if (player_num == 0) return out;
	else return out / player_num;
}

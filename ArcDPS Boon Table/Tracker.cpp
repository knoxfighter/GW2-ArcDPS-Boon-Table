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

void Tracker::addPlayer(ag* src, ag* dst)
{
	if (!is_player(src)) return;

	uintptr_t new_id = src->id;
	std::string new_character_name = std::string(src->name);
	std::string new_account_name = std::string(dst->name);
	uint8_t new_subgroup = dst->team;

	Player* current_player = getPlayer(new_account_name);

	if (current_player)
	{
		current_player->is_relevant = true;
		current_player->id = new_id;
		current_player->name = new_character_name;
		current_player->subgroup = new_subgroup;
	}
	else
	{
		std::unique_lock<std::mutex> lock(players_mtx);
		players.push_back(Player(new_id, new_character_name, new_account_name, new_subgroup));
		lock.unlock();
		bakeCombatData();
	}
}

void Tracker::removePlayer(ag* src)
{
	uintptr_t new_id = src->id;
	std::string new_character_name = std::string(src->name);

	Player* current_player = getPlayer(new_id);

	if (current_player)
	{
		current_player->is_relevant = false;
		if (current_player->in_combat) current_player->combatExit(getCurrentTime());
		bakeCombatData();
	}
}

void Tracker::clearPlayers()
{
	std::unique_lock<std::mutex> lock(players_mtx);

	players.clear();
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

Player* Tracker::getPlayer(uintptr_t new_player)
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find(players.begin(), players.end(), new_player);

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

Player* Tracker::getPlayer(std::string new_player)
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find(players.begin(), players.end(), new_player);

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

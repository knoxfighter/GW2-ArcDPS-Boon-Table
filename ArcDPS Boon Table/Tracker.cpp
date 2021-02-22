#include "Tracker.h"

void Tracker::addPlayer(ag* src, ag* dst)
{
	if (!is_player(src)) return;

	uintptr_t id = src->id;
	uint8_t subgroup = dst->team;
	prof profession = dst->prof;
	std::string characterName = std::string(src->name);
	std::string accountName = std::string(dst->name);
	// remove ':' from accountName if it is there
	if (accountName.at(0) == ':') {
		accountName.erase(0, 1);
	}

	if (characterName.length() < 2) return;
	if (accountName.length() < 2) return;

	Player* current_player = getPlayer(accountName);

	// if player already tracked, just update it
	if (current_player)
	{
		current_player->id = id;
		current_player->name = characterName;
		current_player->subgroup = subgroup;
		current_player->profession = profession;
	}
	else
	{
		std::unique_lock<std::mutex> lock(players_mtx);
		players.emplace_back(id, characterName, accountName, subgroup, profession);
		lock.unlock();

		bakeCombatData();
	}
}

void Tracker::removePlayer(ag* src)
{
	uintptr_t id = src->id;
	std::string characterName = std::string(src->name);

	// remove player from tracked list at all
	Player* current_player = getPlayer(id);

	std::unique_lock<std::mutex> lock(players_mtx);
	players.remove(*current_player);
	lock.unlock();

	bakeCombatData();
}

void Tracker::clearPlayers()
{
	std::unique_lock<std::mutex> lock(players_mtx);
	players.clear();
	lock.unlock();

	bakeCombatData();
}

void Tracker::bakeCombatData()
{
	std::lock_guard<std::mutex> lock(subgroups_mtx);
	subgroups = getSubgroups();

	is_squad = players.size() > 5;
}

Player* Tracker::getPlayer(uintptr_t new_player)
{
	if (!new_player) return nullptr;
	std::lock_guard<std::mutex> lock(players_mtx);
	const auto& it = std::find(players.begin(), players.end(), new_player);

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
	if (new_player.empty()) return nullptr;
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

void Tracker::applyBoon(ag* src, ag* dst, cbtevent* ev)
{
	Player* current_player = nullptr;

	if ((current_player = getPlayer(src->id)) && is_player(dst))
	{
		current_player->gaveBoon(ev);
	}
	if (current_player = getPlayer(dst->id))
	{
		current_player->applyBoon(ev);
	}
}

std::list<uint8_t> Tracker::getSubgroups()
{
	std::lock_guard<std::mutex> lock(players_mtx);
	auto out = std::list<uint8_t>();
	bool found = false;

	for (const Player& player : players) {
		for (uint8_t current_sub : out) {
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

float Tracker::getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (const Player& player : players) {
		if (player.subgroup != subgroup) continue;

		out += player.getBoonUptime(boon);

		// count players in this subgroup
		++player_num;
	}

	if (player_num == 0) {
		return out;
	} else {
		return out / player_num;
	}
}

float Tracker::getAverageBoonUptime(const BoonDef& boon) const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (const Player& player : players) {
		out += player.getBoonUptime(boon);
		player_num++;
	}
	if (isnan(out)) return 0.0f;
	if (player_num == 0) return out;
	else return out / player_num;
}

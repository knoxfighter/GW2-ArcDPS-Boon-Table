#include "Tracker.h"

void Tracker::addPlayer(ag* src, ag* dst)
{
	if (!is_player(src)) return;

	uintptr_t id = src->id;
	uint8_t subgroup = dst->team;
	prof profession = dst->prof;
	std::string characterName = std::string(src->name);
	std::string accountName = std::string(dst->name);

	addPlayer(id, subgroup, profession, characterName, accountName);
}

void Tracker::addPlayer(uintptr_t id,
	uint8_t subgroup,
	prof profession,
	std::string characterName,
	std::string accountName)
{
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
		addNewPlayer(id, subgroup, profession, characterName, accountName);
	}
}

void Tracker::addNewPlayer(uintptr_t id,
	uint8_t subgroup,
	prof profession,
	std::string characterName,
	std::string accountName)
{
	std::unique_lock<std::mutex> lock(players_mtx);
	players.emplace_back(id, characterName, accountName, subgroup, profession);
	lock.unlock();

	bakeCombatData();

	//self has always the id 2000, not sure why, but this is easier than adding a new parameter to everything
	if (id == 2000) {
		self_player = getPlayer(id);
	}
}

void Tracker::addNPC(uintptr_t id,
	std::string name,
	cbtevent* ev)
{
	NPC* old_npc = getNPC(name);
	if (old_npc && old_npc->id != id && old_npc->in_combat) {
		//If there is a new npc id it means that a new instance was spawned, and the combat timer ran for the old instance without it beeing spawned
		// this removes the npc from combat, if its actually in combat it will be set later in this function
		// worst set the combat start to a later point, when the npc got its first buff, which might be later during the fight than you actually could buff it
		getNPC(name)->combatExit(ev);
	}

	addNPC(id, name);
	if(ev)
		if (self_player && self_player->in_combat && getNPC(id)) {
			NPC* npc = getNPC(id);
			if (!npc->in_combat) {
				npc->combatEnter(ev);
			}
		}
}

void Tracker::addNPC(uintptr_t id,
	std::string name)
{
	NPC* current_npc = getNPC(name);

	// if npc already tracked, just update it
	// npcs get tracked by their name, not their id (respawned npcs get new ids)
	if (current_npc)
	{
		current_npc->id = id;
		//current_npc->name = name;
	}
	else
	{
		addNewNPC(id, name);
	}
}

void Tracker::addNewNPC(uintptr_t id,
	std::string name)
{
	std::unique_lock<std::mutex> lock(npcs_mtx);
	npcs.emplace_back(id, name);
	lock.unlock();

	bakeCombatData();
}

void Tracker::removePlayer(ag* src)
{
	uintptr_t id = src->id;
	std::string characterName = std::string(src->name);

	// remove player from tracked list at all
	Player* current_player = getPlayer(id);
	removeEntity(current_player);
}

void Tracker::removeEntity(Entity* entity) {
	//TODO: implement
}
void Tracker::removeEntity(NPC* npc) {
	std::unique_lock<std::mutex> lock(npcs_mtx);
	npcs.remove(*npc);
	lock.unlock();

	bakeCombatData();
}
void Tracker::removeEntity(Player* current_player) {
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

Entity* Tracker::getEntity(uintptr_t new_entity) {
	Entity* entity = getPlayer(new_entity);
	if (entity != nullptr) return entity;
	return getNPC(new_entity);

}

NPC* Tracker::getNPC(uintptr_t new_npc)
{
	if (!new_npc) return nullptr;
	std::lock_guard<std::mutex> lock(npcs_mtx);
	const auto& it = std::find(npcs.begin(), npcs.end(), new_npc);

	//player not tracked yet
	if (it == npcs.end())
	{
		return nullptr;
	}
	else//npc tracked
	{
		return &*it;
	}
}

NPC* Tracker::getNPC(std::string new_name)
{
	if (new_name.empty()) return nullptr;
	std::lock_guard<std::mutex> lock(npcs_mtx);
	auto it = std::find(npcs.begin(), npcs.end(), new_name);

	//player not tracked yet
	if (it == npcs.end())
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
	Entity* current_player = nullptr;

	if ((current_player = getEntity(src->id)) && is_player(dst))
	{
		current_player->gaveBoon(ev);
	}
	if (current_player = getEntity(dst->id))
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

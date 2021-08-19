#include "Tracker.h"

#include "AppChart.h"

Tracker liveTracker;

bool Tracker::isSquad() const {
	return players.size() > 5;
}

void Tracker::addPlayer(ag* src, ag* dst) {
	if (!is_player(src)) return;

	uintptr_t id = src->id;
	uint8_t subgroup = dst->team;
	Prof profession = dst->prof;
	std::string characterName = std::string(src->name);
	std::string accountName = std::string(dst->name);

	addPlayer(id, subgroup, profession, characterName, accountName, dst->self);
}

void Tracker::addPlayer(uintptr_t id,
                        uint8_t subgroup,
                        Prof profession,
                        std::string characterName,
                        std::string accountName,
						bool self = false) {
	// remove ':' from accountName if it is there
	if (accountName.at(0) == ':') {
		accountName.erase(0, 1);
	}

	if (characterName.length() < 2) return;
	if (accountName.length() < 2) return;

	Player* current_player = getPlayer(accountName);

	// if player already tracked, just update it
	if (current_player) {
		current_player->id = id;
		current_player->name = characterName;
		current_player->subgroup = subgroup;
		current_player->profession = profession;
		current_player->self = self;
	} else {
		addNewPlayer(id, subgroup, profession, characterName, accountName, self);
	}
}

void Tracker::addNewPlayer(uintptr_t id,
                           uint8_t subgroup,
                           Prof profession,
                           std::string characterName,
                           std::string accountName,
						   bool self = false) {
	std::unique_lock<std::mutex> lock(players_mtx);
	players.try_emplace(id, id, characterName, accountName, subgroup, profession, self);
	// give charts index to newly player
	charts.addPlayer(id);
	lock.unlock();
}

void Tracker::addNPC(uintptr_t id, std::string name, cbtevent* ev) {
	// FIXME: NPC stuff
	// NPC* npc = getNPC(name);
	// if (npc) {
	// 	if (npc && npc->id != id && npc->in_combat) {
	// 		//If there is a new npc id it means that a new instance was spawned, and the combat timer ran for the old instance without it beeing spawned
	// 		// this removes the npc from combat, if its actually in combat it will be set later in this function
	// 		// worst set the combat start to a later point, when the npc got its first buff, which might be later during the fight than you actually could buff it
	// 		npc->combatExit(ev);
	// 	}
	//
	// 	// if npc already tracked, just update it
	// 	// npcs get tracked by their name, not their id (respawned npcs get new ids)
	// 	npc->id = id;
	// 	//current_npc->name = name;
	// } else {
	// 	// add new npc
	// 	std::unique_lock<std::mutex> lock(npcs_mtx);
	// 	npcs.emplace_back(id, name);
	// 	lock.unlock();
	// }
	//
	// if (ev) {
	// 	Player* self_player = getPlayer(2000);
	// 	if (self_player && self_player->in_combat && getNPC(id)) {
	// 		NPC* npc = getNPC(id);
	// 		if (npc && !npc->in_combat) {
	// 			npc->combatEnter(ev);
	// 		}
	// 	}
	// }
}

void Tracker::removePlayer(ag* src) {
	uintptr_t id = src->id;
	std::string characterName = std::string(src->name);

	std::unique_lock<std::mutex> lock(players_mtx);

	charts.removePlayer(id);

	// remove player from tracked list at all
	players.erase(id);
	lock.unlock();
}

void Tracker::clearPlayers() {
	std::unique_lock<std::mutex> lock(players_mtx);
	players.clear();

	charts.clearPlayers();
	lock.unlock();
}

void Tracker::clearNPCs() {
	std::unique_lock<std::mutex> lock(npcs_mtx);
	npcs.clear();
	lock.unlock();
}

Player* Tracker::getPlayer(uintptr_t new_player) {
	if (!new_player) return nullptr;
	const auto& it = players.find(new_player);

	//player not tracked yet
	if (it == players.end()) {
		return nullptr;
	} else //player tracked
	{
		return &it->second;
	}
}

Entity* Tracker::getEntity(uintptr_t new_entity) {
	Entity* entity = getPlayer(new_entity);
	if (entity != nullptr) return entity;
	return getNPC(new_entity);
}

IEntity* Tracker::getIEntity(uintptr_t new_entity) {
	return getEntity(new_entity);
}

NPC* Tracker::getNPC(uintptr_t new_npc) {
	if (!new_npc) return nullptr;
	std::lock_guard<std::mutex> lock(npcs_mtx);
	const auto& it = std::find(npcs.begin(), npcs.end(), new_npc);

	//player not tracked yet
	if (it == npcs.end()) {
		return nullptr;
	} else //npc tracked
	{
		return &*it;
	}
}

NPC* Tracker::getNPC(std::string new_name) {
	if (new_name.empty()) return nullptr;
	std::lock_guard<std::mutex> lock(npcs_mtx);
	auto it = std::find(npcs.begin(), npcs.end(), new_name);

	//player not tracked yet
	if (it == npcs.end()) {
		return nullptr;
	} else //player tracked
	{
		return &*it;
	}
}

Player* Tracker::getPlayer(std::string new_player) {
	if (new_player.empty()) return nullptr;
	std::lock_guard<std::mutex> lock(players_mtx);
	auto it = std::find_if(players.begin(), players.end(), [&new_player](const auto& player) {
		return player.second == new_player;
	});

	//player not tracked yet
	if (it == players.end()) {
		return nullptr;
	} else //player tracked
	{
		return &it->second;
	}
}

IPlayer* Tracker::getIPlayer(uintptr_t new_player) {
	return getPlayer(new_player);
}

IPlayer* Tracker::getIPlayer(std::string new_player) {
	return getPlayer(new_player);
}

IPlayer* Tracker::getSelfIPlayer() {
	for (auto& player : players) {
		if (player.second.self == true) {
			return &player.second;
		}
	}

	return nullptr;
}

std::unordered_map<uintptr_t, Player>& Tracker::getPlayers() {
	return players;
}

std::set<uintptr_t> Tracker::getAllPlayerIds() {
	std::set<uintptr_t> ret;
	for (const auto& player : players) {
		ret.emplace(player.first);
	}
	return ret;
}

void Tracker::applyBoon(ag* src, ag* dst, cbtevent* ev) {
	Entity* current_player = getEntity(src->id);

	// outgoing boon to other player
	if (current_player && is_player(dst)) {
		current_player->gaveBoon(ev);
	}

	// incoming boon
	current_player = getEntity(dst->id);
	if (current_player) {
		current_player->applyBoon(ev);
	}
}

void Tracker::dealtDamage(ag* src, cbtevent* ev) {
	Entity* entity = getEntity(src->id);

	if (entity) {
		entity->dealtDamage(ev);
	}
}

std::set<uint8_t> Tracker::getSubgroups() const {
	std::set<uint8_t> out;

	for (auto& player : players) {
		out.emplace(player.second.subgroup);
	}
	return out;
}

float Tracker::getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto& pair : players) {
		const Player& player = pair.second;
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

float Tracker::getSubgroupOver90(uint8_t subgroup) const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto& pair : players) {
		const Player& player = pair.second;
		if (player.subgroup != subgroup) continue;

		out += player.getOver90();

		// count players in this subgroup
		++player_num;
	}

	if (player_num == 0) {
		return out;
	}
	else {
		return out / player_num;
	}
}

float Tracker::getAverageBoonUptime(const BoonDef& boon) const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto& pair : players) {
		const Player& player = pair.second;
		out += player.getBoonUptime(boon);
		player_num++;
	}
	if (isnan(out)) return 0.0f;
	if (player_num == 0) return out;
	return out / player_num;
}

float Tracker::getAverageOver90() const {
	float out = 0.0f;
	uint8_t player_num = 0;

	for (auto& pair : players) {
		const Player& player = pair.second;
		out += player.getOver90();
		player_num++;
	}
	if (isnan(out)) return 0.0f;
	if (player_num == 0) return out;
	return out / player_num;
}

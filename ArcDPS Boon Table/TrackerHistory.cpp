#include "TrackerHistory.h"

std::chrono::system_clock::duration TrackerHistory::getDuration() const {
	return logDuration;
}

std::chrono::system_clock::time_point TrackerHistory::getStarttime() const {
	return logStartTimestamp;
}

const std::string& TrackerHistory::getLogName() const {
	return logName;
}

float TrackerHistory::getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const {
	return subgroupUptime.at(subgroup).at(boon.ids[0]);
}

float TrackerHistory::getSubgroupOver90(uint8_t subgroup) const {
	return subgroupOver90.at(subgroup);
}

float TrackerHistory::getAverageBoonUptime(const BoonDef& boon) const {
	return averageUptime.at(boon.ids[0]);
}

float TrackerHistory::getAverageOver90() const {
	return averageOver90;
}

IPlayer* TrackerHistory::getIPlayer(uintptr_t new_player) {
	if (!new_player) return nullptr;
	const auto & pair = players.find(new_player);
	if (pair != players.end()) {
		return &pair->second;
	}
	return nullptr;
}

IPlayer* TrackerHistory::getIPlayer(std::string new_player) {
	if (new_player.empty()) return nullptr;
	std::lock_guard lock(players_mtx);
	auto it = std::ranges::find_if(players, [&new_player](const auto& player) {
		return player.second == new_player;
	});

	//player not tracked yet
	if (it == players.end()) {
		return nullptr;
	}
	//player tracked
	return &it->second;
}

IPlayer* TrackerHistory::getSelfIPlayer() {
	for (auto& player : players) {
		if (player.second.isSelf()) {
			return &player.second;
		}
	}
	return nullptr;
}

std::set<uintptr_t> TrackerHistory::getAllPlayerIds() {
	std::set<uintptr_t> ret;
	for (const auto& player : players) {
		ret.emplace(player.first);
	}
	return ret;
}

IEntity* TrackerHistory::getIEntity(uintptr_t new_entity) {
	// FIXME: Add NPC check
	return getIPlayer(new_entity);
}

bool TrackerHistory::isSquad() const {
	return squad;
}

std::set<uint8_t> TrackerHistory::getSubgroups() const {
	return subgroups;
}

#include "TrackerHistory.h"

uint32_t TrackerHistory::getDuration() const {
	return logDuration;
}

std::chrono::hh_mm_ss<std::chrono::system_clock::duration> TrackerHistory::getStarttime() const {
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

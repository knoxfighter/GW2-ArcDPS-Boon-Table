#pragma once

#include "ITracker.h"
#include "PlayerHistory.h"
#include "Tracker.h"

class TrackerHistory : public ITracker {
private:
	bool squad = false;
	uint32_t logDuration = 0;
	std::chrono::hh_mm_ss<std::chrono::system_clock::duration> logStartTimestamp;
	std::string logName;
	std::set<uint8_t> subgroups;

	// subgroup -> boonID -> value
	std::map<uint8_t, std::map<uint32_t, float>> subgroupUptime;
	// subgroup -> value
	std::map<uint8_t, float> subgroupOver90;
	// boonID -> value
	std::map<uint32_t, float> averageUptime;
	float averageOver90;

	// players
	std::unordered_map<uintptr_t, PlayerHistory> players;

public:
	TrackerHistory(Tracker& tracker, uint32_t logDuration, std::chrono::hh_mm_ss<std::chrono::system_clock::duration> logStarttime, const std::string& logName) : squad(tracker.isSquad()),
		subgroups(tracker.getSubgroups()), logDuration(logDuration), logStartTimestamp(logStarttime), logName(logName) {
		// just save it, ignore the difference in combattime for now (eventually adjust it to logtime)

		// average
		for (const BoonDef& trackedBuff : tracked_buffs) {
			averageUptime[trackedBuff.ids[0]] = tracker.getAverageBoonUptime(
				trackedBuff);

		}
		averageOver90 = tracker.getAverageOver90();

		// subgroups
		for (uint8_t subgroup : tracker.getSubgroups()) {
			for (const BoonDef& trackedBuff : tracked_buffs) {
				float subgroupBoonUptime = tracker.getSubgroupBoonUptime(trackedBuff, subgroup);
				subgroupUptime[subgroup][trackedBuff.ids[0]] = subgroupBoonUptime;

			}

			subgroupOver90[subgroup] = tracker.getSubgroupOver90(subgroup);

		}

		for (auto& player : tracker.getPlayers()) {
			PlayerHistory hp(player.second);
			players[player.second.getId()] = hp;

		}
	}

	[[nodiscard]] uint32_t getDuration() const;
	[[nodiscard]] std::chrono::hh_mm_ss<std::chrono::system_clock::duration> getStarttime() const;
	[[nodiscard]] const std::string& getLogName() const;
	float getSubgroupBoonUptime(const BoonDef& boon, uint8_t subgroup) const override;
	float getSubgroupOver90(uint8_t subgroup) const override;
	float getAverageBoonUptime(const BoonDef& boon) const override;
	float getAverageOver90() const override;
	IPlayer* getIPlayer(uintptr_t new_player) override;
	IPlayer* getIPlayer(std::string new_player) override;
	std::set<uintptr_t> getAllPlayerIds() override;
	IEntity* getIEntity(uintptr_t new_entity) override;
	bool isSquad() const override;
	std::set<uint8_t> getSubgroups() const override;
};

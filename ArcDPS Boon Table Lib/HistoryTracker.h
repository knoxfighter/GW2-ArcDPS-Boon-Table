#pragma once

#include "HistoryPlayer.h"
#include "ITracker.h"

class Tracker;

class HistoryTracker : public ITracker {
public:
	explicit HistoryTracker(const Tracker& pTracker);

	void RunOnPlayers(IPlayerFunc pFunction) const override;
	void RunOnPlayer(uintptr_t pId, IPlayerFunc pFunction) const override;
	void RunOnPlayer(const std::string& pId, IPlayerFunc pFunction) const override;
	[[nodiscard]] const IPlayer* GetPlayer(uintptr_t pId) const override;
	[[nodiscard]] const IPlayer* GetPlayer(const std::string& pId) const override;

	[[nodiscard]] double GetSubgroupIntensity(uint8_t pSubgroup, Boons pBoons) const override;
	[[nodiscard]] double GetSubgroupUptime(uint8_t pSubgroup, Boons pBoons) const override;
	[[nodiscard]] double GetTotalIntensity(Boons pBoons) const override;
	[[nodiscard]] double GetTotalUptime(Boons pBoons) const override;

	[[nodiscard]] uintptr_t GetEncounterId() const override;
	[[nodiscard]] uint64_t GetDuration() const override;

private:
	uintptr_t mCurrentBoss = 0;
	uint64_t mDuration = 0;
	bool mIsWvW = false;

	std::vector<HistoryPlayer> mPlayers;
};

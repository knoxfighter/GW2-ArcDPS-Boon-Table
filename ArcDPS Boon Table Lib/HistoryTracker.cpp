#include "HistoryTracker.h"

#include "Tracker.h"

#include <ranges>

HistoryTracker::HistoryTracker(const Tracker& pTracker) {
	mIsWvW = pTracker.mIsWvW;
	mCurrentBoss = pTracker.mCurrentBoss;
	mDuration = pTracker.GetDuration();

	Tracker::PlayerReadLock guard(pTracker.PlayersMutex);

	for (const auto& player : pTracker.mPlayers) {
		mPlayers.emplace_back(player);
	}
}

void HistoryTracker::RunOnPlayers(IPlayerFunc pFunction) const {
	for (const auto& player : mPlayers) {
		pFunction(player);
	}
}

void HistoryTracker::RunOnPlayer(uintptr_t pId, IPlayerFunc pFunction) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const HistoryPlayer& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		pFunction(*player);
	}
}

void HistoryTracker::RunOnPlayer(const std::string& pId, IPlayerFunc pFunction) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const HistoryPlayer& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		pFunction(*player);
	}
}

const IPlayer* HistoryTracker::GetPlayer(uintptr_t pId) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const HistoryPlayer& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

const IPlayer* HistoryTracker::GetPlayer(const std::string& pId) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const HistoryPlayer& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

double HistoryTracker::GetSubgroupIntensity(uint8_t pSubgroup, Boons pBoons) const {
	double res = 0;
	size_t amount = 0;
	for (const HistoryPlayer& player : mPlayers | std::ranges::views::filter([pSubgroup](const HistoryPlayer& pPlayer){ return pPlayer.GetSubgroup() == pSubgroup; })) {
		res += player.GetIntensity(pBoons);
		++amount;
	}
	return res / amount;
}

double HistoryTracker::GetSubgroupUptime(uint8_t pSubgroup, Boons pBoons) const {
	double res = 0;
	size_t amount = 0;
	for (const HistoryPlayer& player : mPlayers | std::ranges::views::filter([pSubgroup](const HistoryPlayer& pPlayer){ return pPlayer.GetSubgroup() == pSubgroup; })) {
		res += player.GetUptime(pBoons);
		++amount;
	}
	return res / amount;
}

double HistoryTracker::GetTotalIntensity(Boons pBoons) const {
	double res = 0;
	for (const HistoryPlayer& player : mPlayers) {
		res += player.GetIntensity(pBoons);
	}
	return res / mPlayers.size();
}

double HistoryTracker::GetTotalUptime(Boons pBoons) const {
	double res = 0;
	for (const HistoryPlayer& player : mPlayers) {
		res += player.GetUptime(pBoons);
	}
	return res / mPlayers.size();
}

uintptr_t HistoryTracker::GetEncounterId() const {
	return mCurrentBoss;
}

uint64_t HistoryTracker::GetDuration() const {
	return mDuration;
}

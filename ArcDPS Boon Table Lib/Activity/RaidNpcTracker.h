#pragma once

#include "NpcUpdateTracker.h"

#include "../Logger.h"

class RaidNpcTracker : public NpcUpdateTracker {
public:
	RaidNpcTracker(Tracker& pTracker, uint64_t pTime, auto... pIds) : NpcUpdateTracker(pTracker, pTime) {
		RegisterIds(pIds...);
	}
	RaidNpcTracker(Tracker& pTracker, IActivityTracker& pOldTracker, auto... pIds) : NpcUpdateTracker(pTracker, pOldTracker) {
		RegisterIds(pIds...);
	}

	void RegisterIds(auto... pIds) {
		const auto emplacer = [this](auto pElem) {
			mTargetIds.emplace_back(static_cast<int>(pElem));
		};
		(emplacer(pIds),...);
	}
	void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) override;
	void Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) override;

private:
	std::vector<int> mTargetIds;
};

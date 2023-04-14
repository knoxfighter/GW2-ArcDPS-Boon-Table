#pragma once

#include "IActivityTracker.h"

#include <vector>

/**
 * How to use:
 * Create this class instead of `IffActivity` on log start.
 * Call `LogNpcUpdate` on that event instead of changing the Activity.
 */
class TestActivity : public IActivityTracker {
public:
	TestActivity(Tracker& pTracker, uint64_t pLogStart)
		: IActivityTracker(pTracker),
		  mLogStart(pLogStart) {}

	void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) override {
		if (pIff == IFF_FOE) {
			mLastIff = pTime;

			if (mFirstIff == 0) {
				mFirstIff = pTime;
			}
		}
		if (std::ranges::find(mTargetIds, pDst.prof) != mTargetIds.end()) {
			mRaidNpcEnd = pTime;
		}
	}
	void Reward(uint64_t pTime) override {
		mRewardEvent = pTime;
	}

	void LogNpcUpdate(uint64_t pTime) {
		mLogNpcUpdate = pTime;
	}

	void LogNpcUpdate(uint64_t pTime, auto... pArgs) {
		LogNpcUpdate(pTime);

		const auto emplacer = [this](auto pElem) {
			mTargetIds.emplace_back(static_cast<int>(pElem));
		};
		(emplacer(pArgs),...);
	}

private:
	uint64_t mLogStart = 0;
	uint64_t mLogNpcUpdate = 0;
	uint64_t mFirstIff = 0;
	uint64_t mLastIff = 0;
	uint64_t mRewardEvent = 0;
	uint64_t mRaidNpcEnd = 0;

	std::vector<int> mTargetIds;
};

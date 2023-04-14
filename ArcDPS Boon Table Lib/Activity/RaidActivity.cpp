#include "RaidActivity.h"

#include "../Tracker.h"

void RaidActivity::Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) {
	if (mLocked) return;

	if (std::ranges::find(mTargetIds, pDst.prof) != mTargetIds.end()) {
		mLastActiveTime = pTime;

		if (mFirstActiveTime == 0) {
			mFirstActiveTime = pTime;

			mTracker.ResetPlayerBoons(pTime);
		}

		std::string src;
		std::string dst;
		if (pSrc.name) src = pSrc.name;
		if (pDst.name) dst = pDst.name;

		LOG_T("Activity|RaidActivity|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, pIff);

		IActivityTracker::Activity(pTime);
	}
}

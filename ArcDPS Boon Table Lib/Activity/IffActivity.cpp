#include "IffActivity.h"

#include "../Tracker.h"

void IffActivity::Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) {
	if (mLocked) return;

	if (pIff == IFF_FOE) {
		mLastActiveTime = pTime;

		if (mFirstActiveTime == 0) {
			mFirstActiveTime = pTime;

			mTracker.ResetPlayerBoons(pTime);
		}

		IActivityTracker::Activity(pTime);
	}

	std::string src;
	std::string dst;
	if (pSrc.name) src = pSrc.name;
	if (pDst.name) dst = pDst.name;

	LOG_T("IffActivity|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, pIff);
}

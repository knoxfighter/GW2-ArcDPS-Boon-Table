#include "IActivityTracker.h"

#include "../Tracker.h"

#include <string>

void IActivityTracker::Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) {
	if (mLocked) return;

	mLastActiveTime = pTime;

	if (mFirstActiveTime == 0) {
		mFirstActiveTime = pTime;

		mTracker.ResetPlayerBoons(pTime);
	}

	std::string src;
	std::string dst;
	if (pSrc.name) src = pSrc.name;
	if (pDst.name) dst = pDst.name;

	LOG_T("Activity|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, pIff);

	Activity(pTime);
}

void IActivityTracker::Activity(uint64_t pTime) {
	mTracker.SomeActivity(pTime);
}

uint64_t IActivityTracker::GetFirstActiveTime() {
	return mFirstActiveTime;
}

uint64_t IActivityTracker::GetLastActiveTime() {
	return mLastActiveTime;
}

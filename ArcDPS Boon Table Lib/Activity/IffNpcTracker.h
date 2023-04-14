#pragma once

#include "NpcUpdateTracker.h"

#include "../Logger.h"

class IffNpcTracker final : public NpcUpdateTracker {
public:
	IffNpcTracker(Tracker& pTracker, uint64_t pTime)
		: NpcUpdateTracker(pTracker, pTime) {}

	void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) override {
		if (mLocked) return;

		if (pIff == IFF_FOE) {
			mLastActiveTime = pTime;
			IActivityTracker::Activity(pTime);
		}

		std::string src;
		std::string dst;
		if (pSrc.name) src = pSrc.name;
		if (pDst.name) dst = pDst.name;

		LOG_T("IffActivity|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, pIff);
	}
};

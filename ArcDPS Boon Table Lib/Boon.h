#pragma once

#include "BoonDef.h"
#include "Logger.h"

#include "extension/arcdps_structs_slim.h"

#include <mutex>

class Boon final : public BoonDef {
public:

	Boon() : BoonDef() {}
	Boon(BoonDef pBoonDef) : BoonDef(pBoonDef){}

	Boon(const Boon& pOther)
		: BoonDef(pOther),
		  mIntensity(pOther.mIntensity),
		  // mUptime(pOther.mUptime),
		  // mLastUptimeChange(pOther.mLastUptimeChange),
		  mCurrentStacks(pOther.mCurrentStacks) {}

	Boon(Boon&& pOther) noexcept
		: BoonDef(std::move(pOther)),
		  mIntensity(pOther.mIntensity),
		  // mUptime(pOther.mUptime),
		  // mLastUptimeChange(pOther.mLastUptimeChange),
		  mCurrentStacks(pOther.mCurrentStacks) {}

	Boon& operator=(const Boon& pOther) {
		if (this == &pOther)
			return *this;
		std::scoped_lock guard(mBoonMutex, pOther.mBoonMutex);

		BoonDef::operator=(pOther);
		mIntensity = pOther.mIntensity;
		// mUptime = pOther.mUptime;
		// mLastUptimeChange = pOther.mLastUptimeChange;
		mCurrentStacks = pOther.mCurrentStacks;
		return *this;
	}

	Boon& operator=(Boon&& pOther) noexcept {
		if (this == &pOther)
			return *this;
		std::scoped_lock guard(mBoonMutex, pOther.mBoonMutex);
		BoonDef::operator =(std::move(pOther));
		mIntensity = pOther.mIntensity;
		// mUptime = pOther.mUptime;
		// mLastUptimeChange = pOther.mLastUptimeChange;
		mCurrentStacks = pOther.mCurrentStacks;
		return *this;
	}

	void GotBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, uint32_t pDuration, uint32_t pOverstackValue);
	void ExtendBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, int32_t pDuration);
	void RemoveBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, int32_t pDuration, cbtbuffremove pRemoveType, uintptr_t pDstAgent, uint8_t pIff);
	void BeginCount(uint64_t pTime, bool pReset = false);
	void ResetCount(uint64_t pTime);
	void EndCount(uint64_t pBeginTime, uint64_t pCurrentTime, uint64_t pEndTime);
	void Activity(uint64_t pTime, uint64_t pBeginTime, bool pCountActive);

	double GetIntensity(uint64_t pBeginTime, bool pCountActive) const;
	double GetUptime(uint64_t pBeginTime, bool pCountActive) const;

	// TODO remove
	friend class BoonWindowHandler;

private:
	double mIntensity = 0;
	// uint64_t mIntensityTime = 0; // see mDurationLeft

	uint64_t mLastChangeTime = 0;
	// durationLeft is also used as intensityTime!
	uint64_t mDurationLeft = 0;
	uint64_t mActiveTime = 0;
	uint8_t mCurrentStacks = 0;
	double mUptime = 0;
	mutable std::mutex mBoonMutex;
};

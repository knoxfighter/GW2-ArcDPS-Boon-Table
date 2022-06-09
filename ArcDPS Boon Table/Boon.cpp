#include "Boon.h"

#include "Tracker.h"

void Boon::GotBoon(uint64_t pTime, int32_t pDuration) {
	if (StackType == StackType::Multi) {
		mCurrentStacks += 1;
	}
}

void Boon::RemoveBoon(uint64_t pTime, int32_t pDuration, uint8_t pStacksRemoved, cbtbuffremove pRemoveType) {
	// ignore CBTB_ALL for now, every last stack removed gets a MANUAL call as well.
	if (StackType == StackType::Multi) {
		// calculate new current value
		if (mBeginTime && !mEndTime) {
			mLastCalculationUptime = CalcUptime(pTime);
			mLastCalculationTime = pTime;
		}

		if (pRemoveType == CBTB_ALL) {
			// mCurrentStacks -= pStacksRemoved;
		} else {
			// CBTB_MANUAL && CBTB_SINGLE
			mCurrentStacks -= 1;
		}
	}
}

void Boon::BeginCount(uint64_t pTime) {
	mBeginTime = pTime;
	mLastCalculationTime = pTime;
	mLastCalculationUptime = mCurrentStacks;

	mEndTime = 0;
}

void Boon::EndCount(uint64_t pTime) {
	mEndTime = pTime;
}

double Boon::GetIntensity() {
	if (mEndTime) {
		return mLastCalculationUptime;
	}

	if (!mBeginTime || !mLastCalculationTime) {
		return 0;
	}

	return CalcUptime(std::forward<uint64_t>(Tracker::instance().GetTime()));
}

double Boon::CalcUptime(uint64_t pTime) {
	return static_cast<double>(mLastCalculationTime - mBeginTime) * mLastCalculationUptime + static_cast<double>(pTime - mLastCalculationTime) * mCurrentStacks;
}

#include "Boon.h"

#include "Logger.h"
#include "Tracker.h"

// Uptime calculation:
// SUM(BuffApply.addedDuration) - SUM(BuffApply.overstackDuration) - SUM(BuffRemoveAll.removedDuration) - SUM(NonCaughtBuffRemoveSingles.removedDuration)
//
// to quickly summarize:
// - BuffApply, always keep, add value, remove overstackValue
// - BuffExtension, always keep, add value, remove 0
// - BuffRemoveManual, always discard
// - BuffRemoveAll, always keep, add 0, remove value
// - BuffRemoveSingle, discard if dst_agent == 0 && iff == Unknown, add 0, remove value
// dst_agent == 0 && iff == Unknown means:
// - override, which will always have beforehand a BuffApply which overstackValue equals the BuffRemoveSingle's value
// - natural end, indication that the stack ended naturally, value will be "roughly" 0

void Boon::GotBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, uint32_t pDuration, uint32_t pOverstackValue) {
	LOG_T("Boon '{}' of type '{}' was gotten", BoonDef::Boon, StackType);

	std::lock_guard guard(mBoonMutex);

	if (pCountActive) {
		if (StackType == StackType::Single) {
			// add to duration
			mDurationLeft += pDuration;

			// remove overstack
			mDurationLeft -= pOverstackValue;

			// set new changeTime on first stack
			if (mCurrentStacks == 0) {
				mLastChangeTime = pCurrentTime;
			}
		} else if (StackType == StackType::Multi) {
			// add elapsed time * current stacks to uptime
			uint64_t changeTime = pCurrentTime - mLastChangeTime;

			if (mCurrentStacks > 0) {
				mDurationLeft += changeTime * mCurrentStacks;

				// add time to duration
				mActiveTime += changeTime;
			}

			// set new changeTime
			mLastChangeTime = pCurrentTime;
		}
	}

	++mCurrentStacks;
}

void Boon::ExtendBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, int32_t pDuration) {
	std::lock_guard guard(mBoonMutex);

	if (pCountActive) {
		if (StackType == StackType::Single) {
			// no check if we have a stack, assumes to always have a stack, else it wouldn't be able to extend one.
			// add additional duration
			mDurationLeft += pDuration;
		}
	}
}

void Boon::RemoveBoon(bool pCountActive, uint64_t pBeginTime, uint64_t pCurrentTime, int32_t pDuration, cbtbuffremove pRemoveType, uintptr_t pDstAgent, uint8_t pIff) {
	LOG_T("Boon '{}' of type '{}' was removed with strategy {}", BoonDef::Boon, StackType, pRemoveType);

	std::lock_guard guard(mBoonMutex);

	if (pRemoveType == CBTB_SINGLE) {
		if (pCountActive) {
			if (StackType == StackType::Single) {
				// assume we have a stack, else it wouldn't be removed
				// BuffRemoveSingle, discard if dst_agent == 0 && iff == Unknown, add 0, remove value
				if (pDstAgent == 0 && pIff == IFF_UNKNOWN && BoonDef::Boon != Boons::Regeneration) {
					
				} else {
					// remove from duration
					mDurationLeft -= pDuration;
				}
			} else if (StackType == StackType::Multi) {
				// add elapsed time * current stacks to uptime
				uint64_t changeTime = pCurrentTime - mLastChangeTime;

				mDurationLeft += changeTime * mCurrentStacks;

				// add time to duration
				mActiveTime += changeTime;

				// set new changeTime
				mLastChangeTime = pCurrentTime;
			}
			LOG_T("ActiveTime change|{}", mActiveTime);
		}

		--mCurrentStacks;
	} else if (pRemoveType == CBTB_ALL) {
		if (pCountActive) {
			if (StackType == StackType::Single) {
				// don't calculate here, already done in CBTB_MANUAL
				// remove from duration (cast to uint, so i don't get bamboozled by the guardian passive)
				// mDurationLeft -= static_cast<uint32_t>(pDuration);

				// add elapsed time to active time
				// maximal the elapsed time
				mActiveTime += mDurationLeft;

				// set last changed time to current
				mLastChangeTime = pCurrentTime;

				// reset duration
				mDurationLeft = 0;
			} else if (StackType == StackType::Multi) {
				// add elapsed time * current stacks to uptime
				uint64_t changeTime = pCurrentTime - mLastChangeTime;

				mDurationLeft += changeTime * mCurrentStacks;

				// add time to duration
				mActiveTime += changeTime;

				// set new changeTime
				mLastChangeTime = pCurrentTime;
			}
			LOG_T("ActiveTime change|{}", mActiveTime);
		}

		mCurrentStacks = 0;
	} else if (pRemoveType == CBTB_MANUAL) {
		if (StackType == StackType::Single) {
			// reduce manual single events here, to circumvent integer overflows in arcdps
			// remove from duration (cast to uint, so i don't get bamboozled by the guardian passive)
			mDurationLeft -= static_cast<uint32_t>(pDuration);
		}
	}
}

void Boon::Activity(uint64_t pTime, uint64_t pBeginTime, bool pCountActive) {
	std::lock_guard guard(mBoonMutex);

	if (pCountActive) {
		if (StackType == StackType::Single) {
			// recalculate the uptime based on fightDuration and activeDuration
			mUptime = (double)(mActiveTime + std::min(mDurationLeft, pTime - mLastChangeTime)) / (double)(pTime - pBeginTime) * 100;
			// LOG_T("{} (double)({} + std::min({}, {})) / (double)({}) * 100", mUptime, mActiveTime, mDurationLeft, pTime - mLastChangeTime, pTime - pBeginTime);
		} else if (StackType == StackType::Multi) {
			// recalculate uptime and intensity
			uint64_t changeTime = 0;
			if (mCurrentStacks > 0) {
				changeTime = pTime - mLastChangeTime;
			}
			mUptime = (double)(mActiveTime + changeTime) / (double)(pTime - pBeginTime) * 100;
			mIntensity = (double)(mDurationLeft + changeTime * mCurrentStacks) / (double)(pTime - pBeginTime);
		}
	}
}

void Boon::BeginCount(uint64_t pTime, bool pReset) {
	std::lock_guard guard(mBoonMutex);

	// reset durationLeft if pReset or multistack
	if (pReset || StackType == StackType::Multi) {
		// durationLeft is also used as intensityTime!
		mDurationLeft = 0;
	} else if (StackType == StackType::Single && mCurrentStacks > 0) {
		// remove the changeTime here as well
		// get elapsed time
		uint64_t changeTime = pTime - mLastChangeTime;

		// remove elapsed time from duration
		mDurationLeft -= changeTime;
	}

	mIntensity = 0;
	mLastChangeTime = pTime;
	mActiveTime = 0;
	mUptime = 0;

	if (pReset) {
		mCurrentStacks = 0;
	}
	LOG_T("ActiveTime change|{}", mActiveTime);
}

void Boon::ResetCount(uint64_t pTime) {
	BeginCount(pTime, false);
}

void Boon::EndCount(uint64_t pBeginTime, uint64_t pCurrentTime, uint64_t pEndTime) {
}

double Boon::GetIntensity(uint64_t pBeginTime, bool pCountActive) const {
	return mIntensity;
}

double Boon::GetUptime(uint64_t pBeginTime, bool pCountActive) const {
	return mUptime;
}

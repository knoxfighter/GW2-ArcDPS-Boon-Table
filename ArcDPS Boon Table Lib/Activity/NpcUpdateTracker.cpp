#include "NpcUpdateTracker.h"

#include "../Tracker.h"

NpcUpdateTracker::NpcUpdateTracker(Tracker& pTracker, uint64_t pTime): IActivityTracker(pTracker) {
	mFirstActiveTime = pTime;

	// reset boons when this tracker starts (the others are also reset on first event)
	mTracker.ResetPlayerBoons(pTime);
}
NpcUpdateTracker::NpcUpdateTracker(Tracker& pTracker, IActivityTracker& pOldTracker): IActivityTracker(pTracker) {
	mFirstActiveTime = pOldTracker.GetFirstActiveTime();
	mLastActiveTime = pOldTracker.GetLastActiveTime();
}

#pragma once

#include "IActivityTracker.h"

class NpcUpdateTracker : public IActivityTracker {
public:
	NpcUpdateTracker(Tracker& pTracker, uint64_t pTime);
	NpcUpdateTracker(Tracker& pTracker, IActivityTracker& pOldTracker);
};

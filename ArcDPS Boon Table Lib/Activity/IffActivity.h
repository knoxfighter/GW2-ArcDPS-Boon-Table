#pragma once

#include "IActivityTracker.h"

class IffActivity : public IActivityTracker {
public:
	explicit IffActivity(Tracker& pTracker)
		: IActivityTracker(pTracker) {}

	void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) override;
};

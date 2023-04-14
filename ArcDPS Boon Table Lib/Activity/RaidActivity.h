#pragma once

#include "IActivityTracker.h"

#include "../Logger.h"

#include <vector>

class RaidActivity : public IActivityTracker {
public:
	RaidActivity(Tracker& pTracker, auto... pArgs)
		: IActivityTracker(pTracker) {

		const auto emplacer = [this](auto pElem) {
			mTargetIds.emplace_back(static_cast<int>(pElem));
		};
		(emplacer(pArgs),...);
	}

	void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) override;

private:
	std::vector<int> mTargetIds;
};

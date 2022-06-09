#pragma once

#include "BoonDef.h"

#include "extension/arcdps_structs_slim.h"

class Boon : public BoonDef {
public:
	Boon() : BoonDef() {}

	Boon(BoonDef pBoonDef) : BoonDef(pBoonDef) {}

	void GotBoon(uint64_t pTime, int32_t pDuration);
	void RemoveBoon(uint64_t pTime, int32_t pDuration, uint8_t pStacksRemoved, cbtbuffremove pRemoveType);
	void BeginCount(uint64_t pTime);
	void EndCount(uint64_t pTime);
	double GetIntensity();

	// TODO remove
	friend class BoonWindowHandler;

private:
	uint64_t mBeginTime = 0;
	uint64_t mLastCalculationTime = 0;
	double mLastCalculationUptime = 0;
	uint64_t mEndTime = 0;
	uint8_t mCurrentStacks = 0;

	double CalcUptime(uint64_t pTime);
};

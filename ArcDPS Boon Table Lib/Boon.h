#pragma once

#include "BoonDef.h"

#include "extension/arcdps_structs_slim.h"

class Boon : public BoonDef {
public:
	Boon() : BoonDef() {}

	Boon(BoonDef pBoonDef) : BoonDef(pBoonDef) {}

	void GotBoon(uint64_t pTime, int32_t pDuration);
	void RemoveBoon(double pTime, int32_t pDuration, uint8_t pStacksRemoved, cbtbuffremove pRemoveType);
	void BeginCount(double pTime);
	void EndCount(double pTime);
	double GetIntensity() const;

	// TODO remove
	friend class BoonWindowHandler;

private:
	double mBeginTime = 0;
	double mLastCalculationTime = 0;
	double mLastCalculationUptime = 0;
	double mEndTime = 0;
	uint8_t mCurrentStacks = 0;

	double CalcUptime(double pTime) const;
};

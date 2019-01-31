#pragma once
#include <inttypes.h>
#include <mutex>
#include "Helpers.h"
#include "BuffIds.h"

class Boon
{
public:
	int32_t duration = 0;
	BoonDef* def = nullptr;
	uint64_t expected_end_time = 0;
	
	bool operator==(uint32_t other_id);

	Boon(BoonDef* new_def, int32_t new_duration);

	void Apply(int32_t new_duration);
	void Remove(int32_t new_duration);

	int32_t getDuration(uint64_t new_current_time);
	uint64_t getDurationRemaining(uint64_t new_current_time);

	float getUptime(uint64_t new_current_time, uint64_t new_combat_time);
};
#pragma once
#include <inttypes.h>
#include <mutex>
#include "Helpers.h"

class Boon
{
	int32_t duration = 0;
public:
	uint32_t id = 0;
	uint64_t expected_end_time = 0;
	
	bool operator==(uint32_t other_id);

	Boon(uint32_t new_id, int32_t new_duration);

	void Apply(int32_t new_duration);
	void Remove(int32_t new_duration);

	int32_t getDuration(uint64_t new_current_time);
	uint64_t getDurationRemaining(uint64_t new_current_time);
};
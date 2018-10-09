#pragma once
#include <inttypes.h>
#include <mutex>
#include "Helpers.h"

class Boon
{
	int32_t duration;
public:
	uint32_t id;
	uint64_t expected_end_time;
	
	bool operator==(uint32_t other_id);

	Boon();
	Boon(uint32_t new_id, int32_t new_duration);

	~Boon();

	void Apply(int32_t new_duration);
	void Remove(int32_t new_duration);

	int32_t getDuration(uint64_t new_duration);
};
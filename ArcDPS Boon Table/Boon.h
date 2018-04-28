#pragma once
#include <inttypes.h>

class Boon
{
public:
	uint16_t id;
	int32_t duration;
	
	bool operator==(uint16_t other_id);

	Boon();
	Boon(uint16_t new_id, int32_t new_duration);

	~Boon();

	void Apply(int32_t new_duration);
	void Remove(int32_t new_duration);
};
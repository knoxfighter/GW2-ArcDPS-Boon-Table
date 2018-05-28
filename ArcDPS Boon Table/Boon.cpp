#include "Boon.h"



bool Boon::operator==(uint16_t other_id)
{
	return id == other_id;
}

Boon::Boon()
{
	id = 0;
	duration = 0;
	expected_end_time = 0;
}

Boon::Boon(uint16_t new_id, int32_t new_duration)
{
	id = new_id;
	duration = new_duration;
	expected_end_time = getCurrentTime() + new_duration;
}


Boon::~Boon()
{
}

void Boon::Apply(int32_t new_duration)
{
	duration += new_duration;

	if (getCurrentTime() > expected_end_time)
	{
		expected_end_time = getCurrentTime() + new_duration;
	}
	else
	{
		expected_end_time = expected_end_time - getCurrentTime() + new_duration;
	}
}

void Boon::Remove(int32_t new_duration)
{
	duration -= new_duration;
	if (duration < 0) duration = 0;

	if (expected_end_time > getCurrentTime()) expected_end_time -= new_duration;
}

int32_t Boon::getDuration(uint64_t current_time)
{
	int32_t out = duration;
	
	if (current_time < expected_end_time)
	{
		out -= expected_end_time - current_time;
	}

	return out;
}

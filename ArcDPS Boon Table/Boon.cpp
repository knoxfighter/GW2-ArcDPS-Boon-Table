#include "Boon.h"



bool Boon::operator==(uint32_t other_id)
{
	return def
		&& def->id == other_id;
}

Boon::Boon(BoonDef* new_def, int32_t new_duration)
{
	def = new_def;
	duration = new_duration;
	expected_end_time = getCurrentTime() + new_duration;
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
		expected_end_time = expected_end_time + new_duration;
	}
}

void Boon::Remove(int32_t new_duration)
{
	duration -= new_duration;
	if (duration < 0) duration = 0;

	if (expected_end_time > getCurrentTime()) expected_end_time -= new_duration;
}

int32_t Boon::getDuration(uint64_t new_current_time)
{
	int32_t out = duration;
	
	if (new_current_time < expected_end_time)
	{
		out -= getDurationRemaining(new_current_time);
	}

	return out;
}

//returns the duration of the boon still on the player's bar
uint64_t Boon::getDurationRemaining(uint64_t new_current_time)
{
	uint64_t out = 0;
	if (expected_end_time > new_current_time)
	{
		out = expected_end_time - new_current_time;
		if (def 
			&& def->stacking_type == StackingType_intensity)//TODO: is this accurate?
		{
			out = out / 25;
		}
	}
	else
	{
		out = 0;
	}
	return out;
}
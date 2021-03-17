#include "Boon.h"

bool Boon::operator==(uint32_t other_id) const {
	if (!def) return false;

	for (auto current_id = def->ids.cbegin(); current_id != def->ids.cend(); ++current_id)
	{
		if (*current_id == other_id) return true;
	}

	return false;
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

	if (expected_end_time > getCurrentTime()) expected_end_time -= new_duration;
}

uint64_t Boon::getDuration(uint64_t new_current_time) const {
	uint64_t out = duration;
	
	if (new_current_time < expected_end_time)
	{
		out -= getDurationRemaining(new_current_time,0);
	}

	return out;
}

//returns the duration of the boon still on the player's bar
//inaccurate for intensity stacking buffs
uint64_t Boon::getDurationRemaining(uint64_t new_current_time, uint64_t new_combat_duration) const {
	uint64_t out = 0;
	if (expected_end_time > new_current_time)
	{
		if (def
			&& def->stacking_type == StackingType_intensity
			&& new_combat_duration != 0
			&& ((duration / new_combat_duration) != 0))//TODO: is this accurate?
		{
			out = (expected_end_time - new_current_time) / (duration / new_combat_duration);
		}
		else
		{
			out = expected_end_time - new_current_time;
		}
	}
	else
	{
		out = 0;
	}
	return out;
}

float Boon::getUptime(uint64_t new_current_time, uint64_t new_combat_time) const
{
	float out = duration;

	if (expected_end_time > new_current_time)
	{
		if (def
			&& def->stacking_type == StackingType_intensity)//TODO: is this accurate?
		{
			if (new_combat_time != 0 && ((duration / new_combat_time) != 0))
			{
				out -= (expected_end_time - new_current_time) / (duration / new_combat_time);
			}
		}
		else
		{
			out -= expected_end_time - new_current_time;
		}
	}

	out = out / new_combat_time;

	return out;
}

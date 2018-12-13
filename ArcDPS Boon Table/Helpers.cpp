#include "Helpers.h"

uint64_t getCurrentTime()
{
	return current_time;
}

int32_t getBuffApplyDuration(cbtevent * ev)
{
	if (!ev) return 0;

	if (!ev->is_offcycle)//is normal buff apply
	{
		return ev->value - ev->overstack_value;
	}
	else//is buff extension
	{
		return ev->value;
	}
}

uint64_t current_time = 0;
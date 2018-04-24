#include "Boon.h"



Boon::Boon()
{
	id = 0;
	duration = 0;
}

Boon::Boon(uint16_t new_id, int32_t new_duration)
{
	id = new_id;
	duration = new_duration;
}


Boon::~Boon()
{
}

void Boon::Apply(int32_t new_duration)
{
	duration += new_duration;
}

void Boon::Remove(int32_t new_duration)
{
	duration -= new_duration;
}

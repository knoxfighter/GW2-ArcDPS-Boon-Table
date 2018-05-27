#include "Player.h"

std::mutex boons_mtx;

bool Player::operator==(uintptr_t other_id)
{
	return id == other_id;
}

Player::Player()
{
	id = 0;
	name = "";
	enter_combat_time = getCurrentTime();
	in_combat = false;
	subgroup = 1;
}

Player::Player(uintptr_t new_id, std::string new_name)
{
	id = new_id;
	name = new_name;
	enter_combat_time = getCurrentTime();
	in_combat = false;
	subgroup = 1;
}

Player::~Player()
{

}

void Player::applyBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
	if (ev->value <= ev->overstack_value) return;
	if (!isTrackedBoon(ev->skillid)) return;
	if (!in_combat) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (std::list<Boon>::iterator boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == ev->skillid)
		{
			boon->Apply(ev->value);
			return;
		}
	}

	boons.push_back(Boon(ev->skillid, ev->value));
}

void Player::removeBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
	if (ev->value <= ev->overstack_value) return;
	if (!isTrackedBoon(ev->skillid)) return;
	if (!in_combat) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (std::list<Boon>::iterator boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == ev->skillid)
		{
			boon->Remove(ev->value);
			return;
		}
	}
}

float Player::getBoonUptime(BoonDef new_boon)
{
	for (auto current_boon : boons)
	{
		if (getCombatTime() == 0) return 0.0f;
		else if (current_boon.id == new_boon.id)
		{
			float out = (float)current_boon.getDuration() / getCombatTime();

			if (new_boon.is_duration_stacking)
			{
				out = out > 1.0f ? 1.0f : out;
			}
			else
			{
				out = out > 25.0f ? 25.0f : out;
			}
			return out;
		}
	}
	
	return 0.0f;
}

void Player::combatEnter(uint64_t new_time, uint8_t new_subgroup)
{
	enter_combat_time = new_time;
	in_combat = true;
	subgroup = new_subgroup;

	std::lock_guard<std::mutex> lock(boons_mtx);
	boons.clear();
}

void Player::combatExit(uint64_t new_time)
{
	exit_combat_time = new_time;
	in_combat = false;
}

float Player::getCombatTime()
{
	if (in_combat)
	{
		return getCurrentTime() - enter_combat_time;
	}
	else
	{
		return exit_combat_time - enter_combat_time;
	}
}

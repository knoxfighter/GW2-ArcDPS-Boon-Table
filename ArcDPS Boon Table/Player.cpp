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
	is_relevant = true;
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

bool Player::isRelevant()
{
	return is_relevant;
}

void Player::applyBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
//	if (ev->value <= ev->overstack_value) return;
	if (!isTrackedBoon(ev->skillid)) return;
	if (!in_combat) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (auto boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == ev->skillid)
		{
			boon->Apply(ev->value - ev->overstack_value);
			return;
		}
	}

	boons.push_back(Boon(ev->skillid, ev->value - ev->overstack_value));
}

void Player::removeBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
//	if (ev->value <= ev->overstack_value) return;
	if (!isTrackedBoon(ev->skillid)) return;
	if (!in_combat) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (auto boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == ev->skillid)
		{
			boon->Remove(ev->value);
			return;
		}
	}
}

float Player::getBoonUptime(BoonDef* new_boon)
{
	for (auto current_boon = boons.begin(); current_boon != boons.end(); ++current_boon)
	{
		if (getCombatTime() == 0) return 0.0f;
		else if (current_boon->id == new_boon->id)
		{
			float out = (float)current_boon->getDuration(in_combat ? getCurrentTime() : exit_combat_time) / getCombatTime();

			if (new_boon->is_duration_stacking)
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

bool Player::hasBoonNow(BoonDef * new_boon)
{
	if (!in_combat) return false;

	for (auto current_boon = boons.begin(); current_boon != boons.end(); ++current_boon)
	{
		if (current_boon->id == new_boon->id)
		{
			return current_boon->expected_end_time > current_time;
		}
	}
	return false;
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

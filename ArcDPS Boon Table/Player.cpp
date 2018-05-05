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
}

Player::Player(uintptr_t new_id, std::string new_name)
{
	id = new_id;
	name = new_name;
	enter_combat_time = getCurrentTime();
	in_combat = false;
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

	boons.push_back(Boon(ev->skillid, ev->value - ev->overstack_value));
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

float Player::getBoonUptime(uint16_t new_id)
{
	for (auto boon : boons)
	{
		if (getCombatTime() == 0) return 0.0f;
		else if (boon.id == new_id) return (float)boon.duration / getCombatTime();
	}
	
	return 0.0f;
}

void Player::combatEnter(uint64_t new_time)
{
	enter_combat_time = new_time;
	in_combat = true;

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

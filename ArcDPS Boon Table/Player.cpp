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

void Player::applyBoon(uint16_t new_id, int32_t new_duration)
{
	if (new_duration == 0) return;
	if (!isTrackedBoon(new_id)) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (std::list<Boon>::iterator boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == new_id)
		{
			boon->Apply(new_duration);
			return;
		}
	}

	boons.push_back(Boon(new_id, new_duration));
}

void Player::removeBoon(uint16_t new_id, int32_t new_duration)
{
	if (new_duration == 0) return;
	if (!isTrackedBoon(new_id)) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (std::list<Boon>::iterator boon = boons.begin(); boon != boons.end(); ++boon)
	{
		if (boon->id == new_id)
		{
			boon->Remove(new_duration);
			return;
		}
	}
}

float Player::getBoonUptime(uint16_t new_id)
{
	for (auto boon : boons)
	{
		if (boon.id == new_id) return (float)boon.duration / getCombatTime();
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
	enter_combat_time = new_time;
	in_combat = false;
}

float Player::getCombatTime()
{
	return getCurrentTime() - enter_combat_time;
}

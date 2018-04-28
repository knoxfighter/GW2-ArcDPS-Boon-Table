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
	enter_combat_time = 0;
	in_combat = false;
}

Player::Player(uintptr_t new_id, std::string new_name)
{
	id = new_id;
	name = new_name;
	enter_combat_time = 0;
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

	for (auto boon : boons)
	{
		if(boon.id == new_id) return boon.Apply(new_duration);
	}

	boons.push_back(Boon(new_id, new_duration));
}

void Player::removeBoon(uint16_t new_id, int32_t new_duration)
{
	if (new_duration == 0) return;
	if (!isTrackedBoon(new_id)) return;

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (auto boon : boons)
	{
		if(boon.id == new_id) return boon.Remove(new_duration);
	}
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

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
//	boons = std::list<Boon>();
}

Player::Player(uintptr_t new_id, std::string new_name)
{
	id = new_id;
	name = new_name;
//	boons = std::list<Boon>();
}

Player::~Player()
{

}

void Player::applyBoon(uint16_t new_id, int32_t new_duration)
{
	if (new_duration == 0) return;

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

	std::lock_guard<std::mutex> lock(boons_mtx);

	for (auto boon : boons)
	{
		if(boon.id == new_id) return boon.Remove(new_duration);
	}
}

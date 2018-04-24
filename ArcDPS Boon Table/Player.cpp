#include "Player.h"

bool Player::operator==(uintptr_t other_id)
{
	return id == other_id;
}

Player::Player()
{
	id = 0;
	name = "";
	boons = std::list<Boon>();
}

Player::Player(uintptr_t new_id, std::string new_name)
{
	id = new_id;
	name = new_name;
	boons = std::list<Boon>();
}

Player::~Player()
{

}

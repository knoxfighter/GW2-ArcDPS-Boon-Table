#include "Player.h"

std::mutex boons_mtx;

bool Player::operator==(uintptr_t other_id)
{
	return id == other_id;
}

bool Player::operator==(std::string other_name)
{
	return name == other_name
		|| account_name == other_name;
}

Player::Player(uintptr_t new_id, std::string new_name, std::string new_account_name, uint8_t new_subgroup)
{
	id = new_id;
	name = new_name;
	account_name = new_account_name;
	enter_combat_time = getCurrentTime();
	in_combat = false;
	subgroup = new_subgroup;
	is_relevant = true;
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

	Boon* current_boon = getBoon(ev->skillid);

	if (current_boon)
	{
		current_boon->Apply(getBuffApplyDuration(ev));
	}
	else
	{
		std::lock_guard<std::mutex> lock(boons_mtx);
		boons.push_back(Boon(ev->skillid, getBuffApplyDuration(ev)));
	}
}

void Player::removeBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
//	if (ev->value <= ev->overstack_value) return;
	if (!isTrackedBoon(ev->skillid)) return;
	if (!in_combat) return;

	Boon* current_boon = getBoon(ev->skillid);

	if (current_boon)
	{
		current_boon->Remove(ev->value);
	}
}

Boon* Player::getBoon(uint32_t new_boon)
{
	std::lock_guard<std::mutex> lock(boons_mtx);
	auto it = std::find(boons.begin(), boons.end(), new_boon);

	//boon not tracked
	if (it == boons.end())
	{
		return nullptr;
	}
	else//boon tracked
	{
		return &*it;
	}
}

float Player::getBoonUptime(BoonDef* new_boon)
{
	if (getCombatTime() == 0) return 0.0f;

	Boon* current_boon = getBoon(new_boon->id);

	if (current_boon)
	{
		double out = (double)current_boon->getDuration(in_combat ? getCurrentTime() : exit_combat_time) / (double)getCombatTime();

		switch (new_boon->stacking_type)
		{
		case StackingType_duration:
			out = out > 1.0f ? 1.0f : out;
			break;
		case StackingType_intensity:
			out = out > 25.0f ? 25.0f : out;
			break;
		case StackingType_single:
			out = out > 1.0f ? 1.0f : out;
			break;
		default:
			out = out > 1.0f ? 1.0f : out;
			break;
		}

		if (out < 0.0f) out = 0.0f;

		return out;
	}
	
	return 0.0f;
}

bool Player::hasBoonNow(BoonDef * new_boon)
{
	if (!in_combat) return false;

	Boon* current_boon = getBoon(new_boon->id);

	if (current_boon)
	{
		return current_boon->expected_end_time > getCurrentTime();
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

uint64_t Player::getCombatTime()
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

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

void Player::applyBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
//	if (ev->value <= ev->overstack_value) return;

	BoonDef* current_def = getTrackedBoon(ev->skillid);
	if (!current_def) return;
	
	auto current_boon_list = in_combat ? &boons_uptime : &boons_uptime_initial;

	std::lock_guard<std::mutex> lock(boons_mtx);

	auto it = current_boon_list->find(ev->skillid);

	if (it != current_boon_list->end())
	{
		it->second.Apply(getBuffApplyDuration(ev));
	}
	else
	{
		current_boon_list->insert({ current_def->ids[0],Boon(current_def, getBuffApplyDuration(ev)) });
	}
}

void Player::removeBoon(cbtevent* ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
//	if (ev->value <= ev->overstack_value) return;
	if (!getTrackedBoon(ev->skillid)) return;

	auto current_boon_list = &boons_uptime;
	if (!in_combat)
		current_boon_list = &boons_uptime_initial;

	std::lock_guard<std::mutex> lock(boons_mtx);
	
	auto it = current_boon_list->find(ev->skillid);

	if (it != current_boon_list->end())
	{
		it->second.Remove(ev->value);
	}
}

void Player::gaveBoon(cbtevent * ev)
{
	if (!ev) return;
	if (ev->value == 0) return;
	if (ev->is_offcycle) return;//don't track boon extensions until they have a proper src

	BoonDef* current_def = getTrackedBoon(ev->skillid);
	if (!current_def) return;

	auto current_boon_list = in_combat ? &boons_generation : &boons_generation_initial;

	std::lock_guard<std::mutex> lock(boons_mtx);

	auto it = current_boon_list->find(ev->skillid);
	
	if (it != current_boon_list->end())
	{
		it->second.Apply(getBuffApplyDuration(ev));
	}
	else
	{
		current_boon_list->insert({ current_def->ids[0],Boon(current_def, getBuffApplyDuration(ev)) });
	}
}

double Player::getBoonUptime(BoonDef* new_boon)
{
	if (getCombatTime() == 0) return 0.0f;

	auto it = boons_uptime.find(new_boon->ids[0]);

	if (it != boons_uptime.end())
	{
		double out = (double)it->second.getUptime(in_combat ? getCurrentTime() : exit_combat_time,getCombatTime());

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

double Player::getBoonGeneration(BoonDef * new_boon)
{
	if (getCombatTime() == 0) return 0.0f;

	auto it = boons_generation.find(new_boon->ids[0]);

	if (it != boons_generation.end())
	{
		double out = (double)it->second.duration / getCombatTime();

		if (out < 0.0f) out = 0.0f;

		return out;
	}

	return 0.0f;
}

void Player::combatEnter(cbtevent* ev)
{
	if (!ev) return;
	enter_combat_time = ev->time;
	in_combat = true;
	subgroup = ev->dst_agent;

	std::lock_guard<std::mutex> lock(boons_mtx);
	boons_uptime.clear();

	for (auto current_initial_boon = boons_uptime_initial.begin(); current_initial_boon != boons_uptime_initial.end(); ++current_initial_boon)
	{
		if (current_initial_boon->second.getDurationRemaining(ev->time) > 0)
		{
			boons_uptime.insert({ current_initial_boon->second.def->ids[0], Boon(current_initial_boon->second.def, current_initial_boon->second.getDurationRemaining(ev->time)) });
		}
	}

	boons_uptime_initial.clear();

	boons_generation.clear();

	for (auto current_initial_boon = boons_generation_initial.begin(); current_initial_boon != boons_generation_initial.end(); ++current_initial_boon)
	{
		if (current_initial_boon->second.getDurationRemaining(ev->time) > 0)
		{
			boons_generation.insert({ current_initial_boon->second.def->ids[0], Boon(current_initial_boon->second.def, current_initial_boon->second.getDurationRemaining(ev->time)) });
		}
	}

	boons_generation_initial.clear();
}

void Player::combatExit()
{
	exit_combat_time = getCurrentTime();
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

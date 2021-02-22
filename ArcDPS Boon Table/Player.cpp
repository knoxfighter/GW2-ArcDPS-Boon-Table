#include "Player.h"

std::mutex boons_mtx;

bool Player::operator==(uintptr_t other_id) const {
	return id == other_id;
}

bool Player::operator==(std::string other_name) const {
	return name == other_name
		|| account_name == other_name;
}

bool Player::operator==(const Player& other) const {
	return id == other.id && name == other.name;
}

Player::Player(uintptr_t new_id, const std::string& new_name, const std::string& new_account_name, uint8_t new_subgroup, prof new_profession)
{
	id = new_id;
	name = new_name;
	account_name = new_account_name;
	enter_combat_time = getCurrentTime();
	in_combat = false;
	subgroup = new_subgroup;
	profession = new_profession;
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

void Player::flushAllBoons()
{
	std::lock_guard<std::mutex> lock(boons_mtx);

	boons_uptime_initial.clear();
	boons_uptime.clear();
	boons_generation_initial.clear();
	boons_generation.clear();
}

float Player::getBoonUptime(const BoonDef& boon) const {
	if (getCombatDuration() == 0) return 0.0f;

	auto it = boons_uptime.find(boon.ids[0]);

	if (it != boons_uptime.end())
	{
		float out = it->second.getUptime(in_combat ? getCurrentTime() : exit_combat_time,getCombatDuration());

		switch (boon.stacking_type)
		{
		case StackingType_single:
		case StackingType_duration:
			out = out > 1.0f ? 1.0f : out;
			break;
		case StackingType_intensity:
			out = out > 25.0f ? 25.0f : out;
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

float Player::getBoonGeneration(const BoonDef& new_boon) const {
	if (getCombatDuration() == 0) return 0.0f;

	auto it = boons_generation.find(new_boon.ids[0]);

	if (it != boons_generation.end())
	{
		float out = it->second.duration / getCombatDuration();

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
	uint64_t duration_remaining = 0;

	std::lock_guard<std::mutex> lock(boons_mtx);
	boons_uptime.clear();

	for (auto current_initial_boon = boons_uptime_initial.begin(); current_initial_boon != boons_uptime_initial.end(); ++current_initial_boon)
	{
		duration_remaining = current_initial_boon->second.getDurationRemaining(ev->time, 0);
		if (duration_remaining > 0)
		{
			boons_uptime.insert({ current_initial_boon->second.def->ids[0], Boon(current_initial_boon->second.def, duration_remaining) });
		}
	}

	boons_uptime_initial.clear();

	boons_generation.clear();

	for (auto current_initial_boon = boons_generation_initial.begin(); current_initial_boon != boons_generation_initial.end(); ++current_initial_boon)
	{
		duration_remaining = current_initial_boon->second.getDurationRemaining(ev->time, 0);
		if (duration_remaining > 0)
		{
			boons_generation.insert({ current_initial_boon->second.def->ids[0], Boon(current_initial_boon->second.def, duration_remaining) });
		}
	}

	boons_generation_initial.clear();
}

void Player::combatExit(cbtevent* ev)
{
	exit_combat_time = getCurrentTime();
	in_combat = false;
	uint64_t duration_remaining = 0;

	if (!ev) return;
	std::lock_guard<std::mutex> lock(boons_mtx);
	boons_uptime_initial.clear();

	for (auto current_boon = boons_uptime.begin(); current_boon != boons_uptime.end(); ++current_boon)
	{
		duration_remaining = current_boon->second.getDurationRemaining(ev->time, getCombatDuration());
		if (duration_remaining > 0)
		{
			boons_uptime_initial.insert({ current_boon->second.def->ids[0], Boon(current_boon->second.def, duration_remaining) });
		}
	}

	boons_generation_initial.clear();

	for (auto current_boon = boons_generation.begin(); current_boon != boons_generation.end(); ++current_boon)
	{
		duration_remaining = current_boon->second.getDurationRemaining(ev->time, getCombatDuration());
		if (duration_remaining > 0)
		{
			boons_generation_initial.insert({ current_boon->second.def->ids[0], Boon(current_boon->second.def, duration_remaining) });
		}
	}

	boons_generation.clear();
}

uint64_t Player::getCombatDuration() const {
	if (in_combat)
	{
		return getCurrentTime() - enter_combat_time;
	}
	else
	{
		return exit_combat_time - enter_combat_time;
	}
}

ImVec4 Player::getProfessionColor() const {
	/* e5 writes out colour array ptrs, sizeof(out) == sizeof(ImVec4*) * 5.  [ void e5(ImVec4** out) ]
       out[0] = core cols
                   enum n_colours_core {
                     CCOL_TRANSPARENT,
                     CCOL_WHITE,
                     CCOL_LWHITE,
                     CCOL_LGREY,
                     CCOL_LYELLOW,
                     CCOL_LGREEN,
                     CCOL_LRED,
                     CCOL_LTEAL,
                     CCOL_MGREY,
                     CCOL_DGREY,
                     CCOL_NUM
                   };
       out[1] = prof colours base
       out[2] = prof colours highlight
                   prof colours match prof enum
       out[3] = subgroup colours base
       out[4] = subgroup colours highlight
                   subgroup colours match subgroup, up to game max, out[3][15]
	 */
	ImVec4* arc_colors[5];
	arc_export_e5(arc_colors);

	return arc_colors[2][profession];
}

#include "EntityHistory.h"

#include "Helpers.h"

bool EntityHistory::operator==(std::string other_name) const {
	return other_name == name;
}

uintptr_t EntityHistory::getId() const {
	return id;
}

const std::string& EntityHistory::getName() const {
	return name;
}

float EntityHistory::getOver90() const {
	return over90;
}

ImVec4 EntityHistory::getColor() const {
	ImVec4* arc_colors[5];
	arc_export_e5(arc_colors);

	return arc_colors[1][PROF_UNKNOWN];
}

uint64_t EntityHistory::getCombatDuration() const {
	return combatDuration;
}

float EntityHistory::getBoonUptime(const BoonDef& boon) const {
	auto pair = boonUptime.find(boon.ids[0]);
	if (pair == boonUptime.end()) {
		return 0.f;
	}

	return pair->second;
}

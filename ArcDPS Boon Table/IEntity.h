#pragma once

#include <charconv>

#include "imgui/imgui.h"
#include "Boon.h"

class IEntity {
public:
	virtual uintptr_t getId() const = 0;
	virtual const std::string& getName() const = 0;
	virtual uint64_t getCombatDuration() const = 0;
	virtual float getOver90() const = 0;
	virtual float getBoonUptime(const BoonDef& boon) const = 0;

	virtual ImVec4 getColor() const = 0;
};


#pragma once

#include "Entity.h"

class Tracker;

class NPC : public Entity {
	friend Tracker;
	
public:
	NPC(uintptr_t new_id, const std::string& new_name);

	ImVec4 getColor() const override;
};


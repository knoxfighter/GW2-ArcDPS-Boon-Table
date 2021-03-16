#pragma once
#include "Entity.h"
class NPC :
    public Entity
{
public:
	NPC(uintptr_t new_id, const std::string& new_name);

	ImVec4 getColor() const;

	bool operator==(uintptr_t other_id) const;
	bool operator==(std::string other_name) const;
	bool operator==(const Entity& other) const;
};


#pragma once
#include "Entity.h"
class NPC :
    public Entity
{
public:
	NPC(uintptr_t new_id, const std::string& new_name);

	ImVec4 getColor() const override;
};


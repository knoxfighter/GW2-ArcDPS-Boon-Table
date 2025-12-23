#pragma once

#include <cstdint>

#include "IEntity.h"
#include <ArcdpsExtension/arcdps_structs_slim.h>

class IPlayer : public virtual IEntity{
public:
	[[nodiscard]] virtual uint8_t getSubgroup() const = 0;
	[[nodiscard]] virtual bool isSelf() const = 0;
	[[nodiscard]] virtual Prof getProfession() const = 0;
};

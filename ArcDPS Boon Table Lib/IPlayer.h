#pragma once

#include "BoonDef.h"

class IPlayer {
public:
	virtual ~IPlayer() = default;
	[[nodiscard]] virtual double GetIntensity(Boons pBoons) const = 0;
	[[nodiscard]] virtual double GetUptime(Boons pBoons) const = 0;

	[[nodiscard]] virtual uint8_t GetSubgroup() const = 0;

	[[nodiscard]] virtual const std::string& GetAccountName() const = 0;
	[[nodiscard]] virtual const std::string& GetCharacterName() const = 0;
};

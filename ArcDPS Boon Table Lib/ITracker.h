#pragma once

#include "IPlayer.h"

#include <functional>
#include <string>

class ITracker {
public:
	virtual ~ITracker() = default;

	using IPlayerFunc = std::function<void(const IPlayer&)>;
	virtual void RunOnPlayers(IPlayerFunc pFunction) const = 0;
	virtual void RunOnPlayer(uintptr_t pId, IPlayerFunc pFunction) const = 0;
	virtual void RunOnPlayer(const std::string& pId, IPlayerFunc pFunction) const = 0;

	[[nodiscard]] virtual const IPlayer* GetPlayer(uintptr_t pId) const = 0;
	[[nodiscard]] virtual const IPlayer* GetPlayer(const std::string& pId) const = 0;

	[[nodiscard]] virtual double GetSubgroupIntensity(uint8_t pSubgroup, Boons pBoons) const = 0;
	[[nodiscard]] virtual double GetSubgroupUptime(uint8_t pSubgroup, Boons pBoons) const = 0;
	[[nodiscard]] virtual double GetTotalIntensity(Boons pBoons) const = 0;
	[[nodiscard]] virtual double GetTotalUptime(Boons pBoons) const = 0;

	[[nodiscard]] virtual uintptr_t GetEncounterId() const = 0;
	[[nodiscard]] virtual uint64_t GetDuration() const = 0;
};

#pragma once

#include "EntityHistory.h"
#include "IPlayer.h"
#include "Player.h"

class PlayerHistory : public IPlayer, public EntityHistory {
protected:
	uint8_t subgroup = 0;
	bool self = false;
	Prof prof = PROF_UNKNOWN;
	
public:
	PlayerHistory() = default;
	PlayerHistory(const Player& player) : EntityHistory(player) {
		subgroup = player.getSubgroup();
		self = player.isSelf();
		prof = player.getProfession();
	}

	[[nodiscard]] uint8_t getSubgroup() const override;
	[[nodiscard]] bool isSelf() const override;
	[[nodiscard]] Prof getProfession() const override;
	ImVec4 getColor() const override;
};

#pragma once

#include "IPlayer.h"

#include <ArcdpsExtension/arcdps_structs_slim.h>

class Player;

class HistoryPlayer : public IPlayer {
public:
	explicit HistoryPlayer(const Player& pPlayer);

	bool operator==(const HistoryPlayer& pOther) const { return mAccountName == pOther.mAccountName; }
	bool operator==(const std::string& pOther) const { return mAccountName == pOther; }
	bool operator==(const uintptr_t pOther) const { return mId == pOther; }

	[[nodiscard]] double GetIntensity(Boons pBoons) const override;
	[[nodiscard]] double GetUptime(Boons pBoons) const override;

	[[nodiscard]] uint8_t GetSubgroup() const override;

	[[nodiscard]] const std::string& GetAccountName() const override;
	[[nodiscard]] const std::string& GetCharacterName() const override;

private:
	uintptr_t mId = 0;
	std::string mAccountName;
	std::string mCharacterName;
	Prof mProfession = PROF_UNKNOWN;
	bool mSelf = false;
	uint8_t mSubgroup = 0;
	bool mCountActive = false;

	struct Boon {
		double Uptime = 0;
		double Intensity = 0;
	};

	std::array<Boon, magic_enum::enum_count<Boons>()> mBoons;
};

#include "HistoryPlayer.h"

#include "Player.h"

HistoryPlayer::HistoryPlayer(const Player& pPlayer) {
	std::lock_guard guard(pPlayer.mMutex);

	mId = pPlayer.mId;
	mAccountName = pPlayer.mAccountName;
	mCharacterName = pPlayer.mCharacterName;
	mProfession = pPlayer.mProfession;
	mSelf = pPlayer.mSelf;
	mSubgroup = pPlayer.mSubgroup;
	mCountActive = pPlayer.mCountActive;

	for (Boons boons : magic_enum::enum_values<Boons>()) {
		mBoons[magic_enum::enum_integer(boons)] = {pPlayer.GetUptime(boons), pPlayer.GetIntensity(boons)};
	}
}

double HistoryPlayer::GetIntensity(Boons pBoons) const {
	return mBoons[magic_enum::enum_integer(pBoons)].Intensity;
}

double HistoryPlayer::GetUptime(Boons pBoons) const {
	return mBoons[magic_enum::enum_integer(pBoons)].Uptime;
}

uint8_t HistoryPlayer::GetSubgroup() const {
	return mSubgroup;
}

const std::string& HistoryPlayer::GetAccountName() const {
	return mAccountName;
}

const std::string& HistoryPlayer::GetCharacterName() const {
	return mCharacterName;
}

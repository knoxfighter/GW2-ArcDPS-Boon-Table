#include "Player.h"

void Player::GotBoon(Boons pBoon, double pTime, int32_t pDuration) {
	mBoons[magic_enum::enum_integer(pBoon)].GotBoon(pTime, pDuration);
}

void Player::RemoveBoon(Boons pBoons, double pTime, int32_t pDuration, uint8_t pStacksRemoved, cbtbuffremove pRemoveType) {
	mBoons[magic_enum::enum_integer(pBoons)].RemoveBoon(pTime, pDuration, pStacksRemoved, pRemoveType);
}

void Player::EnterCombat(cbtevent* pEvent) {
	std::lock_guard guard(mMutex);

	mSubgroup = static_cast<uint8_t>(pEvent->dst_agent);
	mEnterCombatTime = pEvent->time;
	mExitCombatTime = 0;

	for (Boon& boon : mBoons) {
		boon.BeginCount(static_cast<double>(pEvent->time) / 1000);
	}
}

void Player::ExitCombat(cbtevent* pEvent) {
	std::lock_guard guard(mMutex);
	mExitCombatTime = pEvent->time;

	for (Boon& boon : mBoons) {
		boon.EndCount(static_cast<double>(pEvent->time) / 1000);
	}
}

double Player::GetIntensity(Boons pBoons) const {
	return mBoons[magic_enum::enum_integer(pBoons)].GetIntensity();
}

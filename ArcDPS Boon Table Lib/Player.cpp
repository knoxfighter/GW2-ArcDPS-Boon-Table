#include "Player.h"

#include "GlobalObjects.h"
#include "Logger.h"
#include "Tracker.h"

void Player::GotBoon(Boons pBoon, uint64_t pTime, uint32_t pDuration, uint32_t pOverstackValue, uint32_t pStackId) {
	LOG_T("{}|{}|{}|{}|Player '{}' got Boon '{}' with Id '{}'", pTime, pDuration, pOverstackValue, pDuration - pOverstackValue, mAccountName, pBoon, pStackId);
	mBoons[magic_enum::enum_integer(pBoon)].GotBoon(mCountActive, mBeginTime, pTime, pDuration, pOverstackValue);
}

void Player::BuffInitial(Boons pBoon, uint64_t pTime, uint32_t pDuration, uint32_t pOverstackValue, uint32_t pStackId) {
	if (mResetPending) {
		mResetPending = false;

		for (Boon& boon : mBoons) {
			boon.BeginCount(pTime, true);
		}
	}

	GotBoon(pBoon, pTime, pDuration, pOverstackValue, pStackId);
}

void Player::ExtendBoon(Boons pBoon, uint64_t pTime, int32_t pDuration, uint32_t pStackId) {
	LOG_T("{}|{}|Player '{}' extended Boon '{}' with Id '{}'", pTime, pDuration, mAccountName, pBoon, pStackId);
	mBoons[magic_enum::enum_integer(pBoon)].ExtendBoon(mCountActive, mBeginTime, pTime, pDuration);
}

void Player::RemoveBoon(Boons pBoons, uint64_t pTime, int32_t pDuration, cbtbuffremove pRemoveType, uint32_t pStackId, uintptr_t pDstAgent, uint8_t pIff) {
	LOG_T("{}|{}|Player '{}' remove Boon '{}' with type '{}' and Id '{}'", pTime, pDuration, mAccountName, pBoons, pRemoveType, pStackId);
	mBoons[magic_enum::enum_integer(pBoons)].RemoveBoon(mCountActive, mBeginTime, pTime, pDuration, pRemoveType, pDstAgent, pIff);
}

void Player::Activity(uint64_t pTime) {
	for (Boon& boon : mBoons) {
		boon.Activity(pTime, mBeginTime, mCountActive);
	}
}

void Player::ResetAllBoons(uint64_t pTime) {
	std::lock_guard guard(mMutex);

	LOG_T("ResetAllBoons");

	mBeginTime = pTime;

	for (Boon& boon : mBoons) {
		boon.ResetCount(pTime);
	}
}

void Player::LogStart(uint64_t pTime) {
	LOG_T("{}|PlayerLogStart|{}", pTime, mAccountName);

	std::lock_guard guard(mMutex);

	if (!mCountActive) {
		mCountActive = true;
		mBeginTime = pTime;
		mResetPending = true;

		for (Boon& boon : mBoons) {
			boon.BeginCount(pTime);
		}
	}
}

void Player::CombatEnter(uint64_t pTime, uint8_t pSubgroup) {
	LOG_T("{}|PlayerCombatEnter|{}|{}", pTime, mAccountName, pSubgroup);

	std::lock_guard guard(mMutex);

	if (pSubgroup > 0)
		mSubgroup = pSubgroup;

	if (!mCountActive) {
		mCountActive = true;
		mBeginTime = pTime;

		for (Boon& boon : mBoons) {
			boon.BeginCount(pTime);
		}
	}
}

void Player::Begin(uint64_t pTime, uint8_t pSubgroup, bool pReset) {
	LOG_T("{}|PlayerBegin|{}|{}", pTime, mAccountName, pReset);

	std::lock_guard guard(mMutex);

	if (pSubgroup > 0)
		mSubgroup = pSubgroup;

	if (!mCountActive) {
		mCountActive = true;
		mBeginTime = pTime;

		for (Boon& boon : mBoons) {
			boon.BeginCount(pTime, pReset);
		}
	}
}

void Player::End(uint64_t pCurrentTime, uint64_t pLastDamageTime) {
	LOG_T("{}|PlayerEnd|{}", pCurrentTime, mAccountName);

	std::lock_guard guard(mMutex);

	if (mCountActive) {
		mCountActive = false;

		for (Boon& boon : mBoons) {
			boon.EndCount(mBeginTime, pCurrentTime, pLastDamageTime);
		}
	}
}

double Player::GetIntensity(Boons pBoons) const {
	return mBoons[magic_enum::enum_integer(pBoons)].GetIntensity(mBeginTime, mCountActive);
}

double Player::GetUptime(Boons pBoons) const {
	return mBoons[magic_enum::enum_integer(pBoons)].GetUptime(mBeginTime, mCountActive);
}

uint8_t Player::GetSubgroup() const {
	return mSubgroup;
}

const std::string& Player::GetAccountName() const {
	return mAccountName;
}

const std::string& Player::GetCharacterName() const {
	return mCharacterName;
}

void Player::Update(const std::string& pCharacterName, Prof pProfession, uint8_t pSubgroup) {
	mCharacterName = pCharacterName;
	mProfession = pProfession;
	mSubgroup = pSubgroup;
}

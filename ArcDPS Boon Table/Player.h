#pragma once

#include "Boon.h"
#include "BoonDef.h"

#include "extension/arcdps_structs_slim.h"

#include <magic_enum.hpp>

#include <mutex>
#include <string>

/**
 * Move are only allowed, when the holder is read-write locked in itself
 * (aka. don't move any instance of this class anywhere)
 *
 * Copy is possible relatively easily. The object itself will be locked, while copy is performed.
 */
class Player {
public:
	Player() {}

	Player(uintptr_t pId, const std::string& pAccountName, const std::string& pCharacterName, Prof pProfession,
	       bool pSelf, uint8_t pSubgroup)
		: mId(pId),
		  mAccountName(pAccountName),
		  mCharacterName(pCharacterName),
		  mProfession(pProfession),
		  mSelf(pSelf),
		  mSubgroup(pSubgroup) {
		for (const BoonDef& boonDef : BOON_DEFS) {
			mBoons[static_cast<size_t>(boonDef.Boon)] = boonDef;
		}
	}

	Player(const Player& pOther) {
		std::scoped_lock guard(mMutex, pOther.mMutex);
		mId = pOther.mId;
		mAccountName = pOther.mAccountName;
		mCharacterName = pOther.mCharacterName;
		mProfession = pOther.mProfession;
		mSelf = pOther.mSelf;
		mSubgroup = pOther.mSubgroup;
		mInCombat = pOther.mInCombat;
		mEnterCombatTime = pOther.mEnterCombatTime;
		mExitCombatTime = pOther.mExitCombatTime;
		mBoons = pOther.mBoons;
	}

	Player(Player&& pOther) noexcept {
		std::scoped_lock guard(mMutex, pOther.mMutex);

		mId = pOther.mId;
		mAccountName = std::move(pOther.mAccountName);
		mCharacterName = std::move(pOther.mCharacterName);
		mProfession = pOther.mProfession;
		mSelf = pOther.mSelf;
		mSubgroup = pOther.mSubgroup;
		mInCombat = pOther.mInCombat;
		mEnterCombatTime = pOther.mEnterCombatTime;
		mExitCombatTime = pOther.mExitCombatTime;
		mBoons = pOther.mBoons;
	}

	Player& operator=(const Player& pOther) {
		if (this == &pOther)
			return *this;

		std::scoped_lock guard(mMutex, pOther.mMutex);
		mId = pOther.mId;
		mAccountName = pOther.mAccountName;
		mCharacterName = pOther.mCharacterName;
		mProfession = pOther.mProfession;
		mSelf = pOther.mSelf;
		mSubgroup = pOther.mSubgroup;
		mInCombat = pOther.mInCombat;
		mEnterCombatTime = pOther.mEnterCombatTime;
		mExitCombatTime = pOther.mExitCombatTime;
		mBoons = pOther.mBoons;
		return *this;
	}

	Player& operator=(Player&& pOther) noexcept {
		if (this == &pOther)
			return *this;
		std::scoped_lock guard(mMutex, pOther.mMutex);
		mId = pOther.mId;
		mAccountName = std::move(pOther.mAccountName);
		mCharacterName = std::move(pOther.mCharacterName);
		mProfession = pOther.mProfession;
		mSelf = pOther.mSelf;
		mSubgroup = pOther.mSubgroup;
		mInCombat = pOther.mInCombat;
		mEnterCombatTime = pOther.mEnterCombatTime;
		mExitCombatTime = pOther.mExitCombatTime;
		mBoons = pOther.mBoons;
		return *this;
	}

	// TODO remove
	friend class BoonWindowHandler;

	bool operator==(const Player& pOther) const { return mAccountName == pOther.mAccountName; }
	bool operator==(const std::string& pOther) const { return mAccountName == pOther; }
	bool operator==(const uintptr_t pOther) const { return mId == pOther; }

	void GotBoon(Boons pBoon, uint64_t pTime, int32_t pDuration);
	void RemoveBoon(Boons pBoons, uint64_t pTime, int32_t pDuration, uint8_t pStacksRemoved, cbtbuffremove pRemoveType);
	void AppliedBoon(Boons pBoon);
	void EnterCombat(cbtevent* pEvent);
	void ExitCombat(cbtevent* pEvent);

private:
	// bool mCommander = false;
	uintptr_t mId = 0;
	std::string mAccountName;
	std::string mCharacterName;
	Prof mProfession = PROF_UNKNOWN;
	bool mSelf = false;
	uint8_t mSubgroup = 0;
	bool mInCombat = false;
	uint64_t mEnterCombatTime = 0;
	uint64_t mExitCombatTime = 0;
	std::array<Boon, magic_enum::enum_count<Boons>()> mBoons;

	// protect this instance of the player.
	mutable std::mutex mMutex;
};

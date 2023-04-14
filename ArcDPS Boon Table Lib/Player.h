#pragma once

#include "Boon.h"
#include "BoonDef.h"
#include "GlobalObjects.h"
#include "IPlayer.h"

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
class Player : public IPlayer {
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
		mCountActive = pOther.mCountActive;
		mBeginTime = pOther.mBeginTime;
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
		mCountActive = pOther.mCountActive;
		mBeginTime = pOther.mBeginTime;
		mBoons = std::move(pOther.mBoons);
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
		mCountActive = pOther.mCountActive;
		mBeginTime = pOther.mBeginTime;
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
		mCountActive = pOther.mCountActive;
		mBeginTime = pOther.mBeginTime;
		mBoons = std::move(pOther.mBoons);
		return *this;
	}

	// TODO remove
	friend class BoonWindowHandler;

	bool operator==(const Player& pOther) const { return mAccountName == pOther.mAccountName; }
	bool operator==(const std::string& pOther) const { return mAccountName == pOther; }
	bool operator==(const uintptr_t pOther) const { return mId == pOther; }

	void GotBoon(Boons pBoon, uint64_t pTime, uint32_t pDuration, uint32_t pOverstackValue, uint32_t pStackId);
	void BuffInitial(Boons pBoon, uint64_t pTime, uint32_t pDuration, uint32_t pOverstackValue, uint32_t pStackId);
	void ExtendBoon(Boons pBoon, uint64_t pTime, int32_t pDuration, uint32_t pStackId);
	void RemoveBoon(Boons pBoons, uint64_t pTime, int32_t pDuration, cbtbuffremove pRemoveType, uint32_t pStackId, uintptr_t pDstAgent, uint8_t pIff);
	void Activity(uint64_t pTime);

	/**
	 * Reset the current calculated values of all Boons.
	 * The current stacks are kept (use on e.g. StatReset)
	 * @param pTime The new begin time.
	 */
	void ResetAllBoons(uint64_t pTime);

	void LogStart(uint64_t pTime);
	void CombatEnter(uint64_t pTime, uint8_t pSubgroup);

	/**
	 * \param pTime Current time
	 * \param pSubgroup Set the subgroup to this value
	 * \param pReset Reset the current boons to 0, so BuffInitial can work
	 */
	void Begin(uint64_t pTime, uint8_t pSubgroup = 0, bool pReset = false);
	void Begin(uint64_t pTime, bool pReset) {
		Begin(pTime, 0, pReset);
	}
	void End(uint64_t pCurrentTime, uint64_t pLastDamageTime);

	[[nodiscard]] double GetIntensity(Boons pBoons) const override;
	[[nodiscard]] double GetUptime(Boons pBoons) const override;

	[[nodiscard]] uint8_t GetSubgroup() const override;

	[[nodiscard]] const std::string& GetAccountName() const override;
	[[nodiscard]] const std::string& GetCharacterName() const override;

	void Update(const std::string& pCharacterName, Prof pProfession, uint8_t pSubgroup);

	friend Boon;
	friend class HistoryPlayer;

private:
	// bool mCommander = false;
	uintptr_t mId = 0;
	std::string mAccountName;
	std::string mCharacterName;
	Prof mProfession = PROF_UNKNOWN;
	bool mSelf = false;
	uint8_t mSubgroup = 0;
	bool mCountActive = false;
	bool mResetPending = false;
	uint64_t mBeginTime = 0;
	std::array<Boon, magic_enum::enum_count<Boons>()> mBoons;

	// protect this instance of the player.
	mutable std::mutex mMutex;
};

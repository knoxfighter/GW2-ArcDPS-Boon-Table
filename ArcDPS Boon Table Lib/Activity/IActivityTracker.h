#pragma once

#include "extension/arcdps_structs_slim.h"

#include <mutex>

class Tracker;

class IActivityTracker {
public:
	virtual ~IActivityTracker() = default;

	explicit IActivityTracker(Tracker& pTracker)
		: mTracker(pTracker) {}

	virtual void Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff);
	virtual void BuffRemove(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
		Activity(pTime, pSrc, pDst, static_cast<iff>(pEvent->iff));
	}
	virtual void Activation(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
		Activity(pTime, pSrc, pDst, static_cast<iff>(pEvent->iff));
	}
	virtual void BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
		Activity(pTime, pSrc, pDst, static_cast<iff>(pEvent->iff));
	}
	virtual void BuffDamage(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
		Activity(pTime, pSrc, pDst, static_cast<iff>(pEvent->iff));
	}
	virtual void Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
		if (
			pEvent->result == CBTR_NORMAL
			|| pEvent->result == CBTR_CRIT
			|| pEvent->result == CBTR_GLANCE
			|| pEvent->result == CBTR_INTERRUPT
			|| pEvent->result == CBTR_KILLINGBLOW
			|| pEvent->result == CBTR_DOWNED
			|| pEvent->result == CBTR_BREAKBAR
			|| pEvent->result == CBTR_BLOCK // eventually not correct
		) {
			Activity(pTime, pSrc, pDst, static_cast<iff>(pEvent->iff));
		}
	}
	virtual void Reward(uint64_t pTime) {
		// TODO disable this on some bosses (mainly Ai)
		if (mReward) {
			mLocked = true;
			mLastActiveTime = pTime;
			Activity(pTime);
		}
	}

	virtual uint64_t GetFirstActiveTime();
	virtual uint64_t GetLastActiveTime();

	void SetRewardActive(bool pReward) {
		mReward = pReward;
	}

protected:
	uint64_t mLastActiveTime = 0;
	uint64_t mFirstActiveTime = 0;
	Tracker& mTracker;
	// If a "Reward" event happened, we don't want to do anything anymore!
	bool mLocked = false;
	// set if "Reward" events should be handled
	bool mReward = true;

	void Activity(uint64_t pTime);
};

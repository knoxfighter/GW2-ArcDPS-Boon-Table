#include "RaidNpcTracker.h"

void RaidNpcTracker::Activity(uint64_t pTime, const ag& pSrc, const ag& pDst, iff pIff) {
	if (mLocked) return;

	if (std::ranges::find(mTargetIds, pDst.prof & 0xffff) != mTargetIds.end()) {
		mLastActiveTime = pTime;

		std::string src;
		std::string dst;
		if (pSrc.name) src = pSrc.name;
		if (pDst.name) dst = pDst.name;

		LOG_T("Activity|RaidNpcTracker|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, pIff);

		IActivityTracker::Activity(pTime);
	}
}

void RaidNpcTracker::Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
	if (mLocked) return;

	if (
		pEvent->result == CBTR_NORMAL
		|| pEvent->result == CBTR_CRIT
		|| pEvent->result == CBTR_GLANCE
		|| pEvent->result == CBTR_INTERRUPT
		|| pEvent->result == CBTR_KILLINGBLOW
		|| pEvent->result == CBTR_DOWNED
		|| pEvent->result == CBTR_BREAKBAR
	) {
		if (std::ranges::find(mTargetIds, pDst.prof) != mTargetIds.end() || std::ranges::find(mTargetIds, pSrc.prof) != mTargetIds.end()) {
			mLastActiveTime = pTime;

			std::string src;
			std::string dst;
			if (pSrc.name) src = pSrc.name;
			if (pDst.name) dst = pDst.name;

			LOG_T("Activity|RaidNpcTracker|{}|'{}' hit '{}' of type '{}'", pTime, src, dst, static_cast<iff>(pEvent->iff));

			IActivityTracker::Activity(pTime);
		}
	}
}

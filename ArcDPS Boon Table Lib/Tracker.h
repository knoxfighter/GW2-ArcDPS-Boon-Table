#pragma once

#include "Player.h"

#include "extension/arcdps_structs_slim.h"
#include "extension/Singleton.h"

#include <mutex>
#include <shared_mutex>

class Tracker final : public Singleton<Tracker>{
public:
	void Event(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId);

	double GetTime() const;
	void SetTime(double pTime) {
		mCurrentTime = pTime;
	}

	/**
	 * Make sure you have ReadLocked the PlayersMutex before!
	 */
	const std::vector<Player>& GetAllPlayer();

	// TODO remove
	friend class BoonWindowHandler;

	typedef std::shared_mutex PlayerMutexType;
	typedef std::unique_lock<PlayerMutexType>  PlayerWriteLock;
	typedef std::shared_lock<PlayerMutexType>  PlayerReadLock;
	std::shared_mutex PlayersMutex;

private:
	void AddPlayer(ag* pSrc, ag* pDst);
	void RemovePlayer(ag* pDst);
	void BuffAppliedEvent(cbtevent* pEvent, ag* pSrc, ag* pDst);
	void BuffRemoveEvent(cbtevent* pEvent, ag* pSrc);

	std::vector<Player> mPlayers;

	bool mIsWvW = false;
	double mCurrentTime = 0;
};

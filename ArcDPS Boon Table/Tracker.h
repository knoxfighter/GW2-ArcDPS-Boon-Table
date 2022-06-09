#pragma once

#include "Player.h"

#include "extension/arcdps_structs_slim.h"
#include "extension/Singleton.h"

#include <mutex>
#include <shared_mutex>

class Tracker final : public Singleton<Tracker>{
public:
	void Event(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId);

	uint64_t GetTime() const;

	// TODO remove
	friend class BoonWindowHandler;

private:
	void AddPlayer(ag* pSrc, ag* pDst);
	void RemovePlayer(ag* pDst);
	void BuffAppliedEvent(cbtevent* pEvent, ag* pSrc, ag* pDst);
	void BuffRemoveEvent(cbtevent* pEvent, ag* pSrc);

	typedef std::shared_mutex PlayerMutexType;
	typedef std::unique_lock<PlayerMutexType>  PlayerWriteLock;
	typedef std::shared_lock<PlayerMutexType>  PlayerReadLock;
	std::vector<Player> mPlayers;
	std::shared_mutex mPlayersMutex;

	bool mIsWvW = false;
	uint64_t mCurrentTime = 0;
};

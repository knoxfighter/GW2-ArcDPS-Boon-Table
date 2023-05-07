#pragma once

#include "ITracker.h"
#include "Logger.h"
#include "Player.h"

#include <ArcdpsExtension/arcdps_structs_slim.h>
#include <ArcdpsExtension/CombatEventHandler.h>
#include <ArcdpsExtension/Singleton.h>

#include "Activity/IActivityTracker.h"

#include <mutex>
#include <shared_mutex>

class Tracker final : public ITracker, public CombatEventHandler, public Singleton<Tracker> {
public:
	Tracker() = default;

	void RunOnPlayers(IPlayerFunc pFunction) const override;
	void RunOnPlayer(uintptr_t pId, IPlayerFunc pFunction) const override;
	void RunOnPlayer(const std::string& pId, IPlayerFunc pFunction) const override;

	/**
	 * Lock the PlayersMutex before calling this!
	 * A PlayerReadLock is enough.
	 */
	[[nodiscard]] const IPlayer* GetPlayer(uintptr_t pId) const override;
	[[nodiscard]] const IPlayer* GetPlayer(const std::string& pId) const override;

	// Get Uptime of groups and total
	[[nodiscard]] double GetSubgroupIntensity(uint8_t pSubgroup, Boons pBoons) const override;
	[[nodiscard]] double GetSubgroupUptime(uint8_t pSubgroup, Boons pBoons) const override;
	[[nodiscard]] double GetTotalIntensity(Boons pBoons) const override;
	[[nodiscard]] double GetTotalUptime(Boons pBoons) const override;

	[[nodiscard]] uintptr_t GetEncounterId() const override;
	[[nodiscard]] uint64_t GetDuration() const override;

	/**
	 * Reset everything in here (basically destructing and then recreating the object)
	 */
	void Reset();
	/**
	 * Call `ResetAllBoons` on every player.
	 */
	void ResetPlayerBoons(uint64_t pTime);

	[[nodiscard]] uint64_t GetLastEventTime() {
		return mLastEventTime;
	}

	// used by ActivityTrackers
	void SomeActivity(uint64_t pTime);

	// TODO remove
	friend class BoonWindowHandler;

	typedef std::shared_mutex PlayerMutexType;
	typedef std::unique_lock<PlayerMutexType>  PlayerWriteLock;
	typedef std::shared_lock<PlayerMutexType>  PlayerReadLock;
	mutable PlayerMutexType PlayersMutex;

	// friend history
	friend class HistoryTracker;

protected:
	void EnterCombat(uint64_t pTime, uintptr_t pAgentId, uint8_t pSubgroup, const ag& pAgent) override;
	void ExitCombat(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) override;
	void LogStart(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) override;
	void LogEnd(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) override;
	void LogNpcUpdate(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) override;
	void BuffRemove(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) override;
	void Activation(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) override;
	void BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) override;
	void BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId, bool pIsActivity);
	void BuffDamage(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) override;
	void Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) override;
	void AgentAdded(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, uintptr_t pInstanceId, Prof pProfession, uint32_t pElite,
	                bool pSelf, uint16_t pTeam, uint8_t pSubgroup) override;
	void AgentRemoved(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, bool pSelf) override;
	void StatReset(uint64_t pTime) override;
	void BuffInitial(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) override;
	void Reward(uint64_t pTime, uintptr_t pSelfId, uintptr_t pRewardId, int32_t pRewardType) override;

private:
	std::vector<Player> mPlayers;

	std::mutex mThisMutex;
	bool mIsWvW = false;
	uintptr_t mCurrentBoss = 0;
	std::unique_ptr<IActivityTracker> mActivityTracker = nullptr;
	std::vector<std::string> mToRemovePlayers;

	void UpdateActivityTracker(bool pNpcUpdate);

	/**
	 * Lock the PlayersMutex before calling this!
	 * A PlayerReadLock is enough.
	 */
	[[nodiscard]] Player* GetPlayer(uintptr_t pId);
	[[nodiscard]] Player* GetPlayer(const std::string& pId);
};

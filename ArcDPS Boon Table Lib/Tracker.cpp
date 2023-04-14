#include "Tracker.h"

#include "GlobalObjects.h"
#include "History.h"
#include "Logger.h"

#include "Activity/Activities.h"

#include "extension/MobIDs.h"

#include <ranges>
#include <map>

void Tracker::RunOnPlayers(IPlayerFunc pFunction) const {
	PlayerReadLock guard(PlayersMutex);

	for (const auto& player : mPlayers) {
		pFunction(player);
	}
}

void Tracker::RunOnPlayer(uintptr_t pId, IPlayerFunc pFunction) const {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		pFunction(*player);
	}
}

void Tracker::RunOnPlayer(const std::string& pId, IPlayerFunc pFunction) const {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		pFunction(*player);
	}
}

const IPlayer* Tracker::GetPlayer(uintptr_t pId) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

const IPlayer* Tracker::GetPlayer(const std::string& pId) const {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

double Tracker::GetSubgroupIntensity(uint8_t pSubgroup, Boons pBoons) const {
	PlayerReadLock guard(PlayersMutex);

	double res = 0;
	size_t amount = 0;
	for (const Player& player : mPlayers | std::ranges::views::filter([pSubgroup](const Player& pPlayer){ return pPlayer.GetSubgroup() == pSubgroup; })) {
		res += player.GetIntensity(pBoons);
		++amount;
	}
	return res / amount;
}

double Tracker::GetSubgroupUptime(uint8_t pSubgroup, Boons pBoons) const {
	PlayerReadLock guard(PlayersMutex);

	double res = 0;
	size_t amount = 0;
	for (const Player& player : mPlayers | std::ranges::views::filter([pSubgroup](const Player& pPlayer){ return pPlayer.GetSubgroup() == pSubgroup; })) {
		res += player.GetUptime(pBoons);
		++amount;
	}
	return res / amount;
}

double Tracker::GetTotalIntensity(Boons pBoons) const {
	PlayerReadLock guard(PlayersMutex);

	double res = 0;
	for (const Player& player : mPlayers) {
		res += player.GetIntensity(pBoons);
	}
	return res / mPlayers.size();
}

double Tracker::GetTotalUptime(Boons pBoons) const {
	PlayerReadLock guard(PlayersMutex);

	double res = 0;
	for (const Player& player : mPlayers) {
		res += player.GetUptime(pBoons);
	}
	return res / mPlayers.size();
}

uintptr_t Tracker::GetEncounterId() const {
	return mCurrentBoss;
}

uint64_t Tracker::GetDuration() const {
	if (mActivityTracker) {
		return mActivityTracker->GetLastActiveTime() - mActivityTracker->GetFirstActiveTime();
	}
	return 0;
}

void Tracker::Reset() {
	PlayerWriteLock guard(PlayersMutex);
	std::lock_guard lock(mThisMutex);

	mPlayers.clear();

	mIsWvW = false;
	mActivityTracker.reset();
	mCurrentBoss = 0;

	CombatEventHandler::Reset();
}

void Tracker::ResetPlayerBoons(uint64_t pTime) {
	PlayerReadLock guard(PlayersMutex);
	for (Player& player : mPlayers) {
		player.ResetAllBoons(pTime);
	}
}

void Tracker::EnterCombat(uint64_t pTime, uintptr_t pAgentId, uint8_t pSubgroup, const ag& pAgent) {
	LOG_T("Player '{}' entered combat", pAgent.name);

	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pAgentId](const auto& pPlayer) {
		return pPlayer == pAgentId;
	});

	if (player != mPlayers.end()) {
		player->CombatEnter(pTime, pSubgroup);
	}
}

void Tracker::ExitCombat(uint64_t pTime, uintptr_t pAgentId, const ag& pAgent) {
	LOG_T("Player '{}' exited combat", pAgent.name);

	// don't run when begin was called by `LogStart`
	if (mCurrentBoss > 0) {
		// SomeActivity(pTime, pAgent, {}, IFF_FOE);
		return;
	}

	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pAgentId](const auto& pPlayer) {
		return pPlayer == pAgentId;
	});

	if (player != mPlayers.end()) {
		player->End(pTime, 0);
	}
}

void Tracker::LogStart(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
	LOG_T("{}|{}|LogStart", pTime, pSpeciesId);

	std::unique_lock lock(mThisMutex);
	mCurrentBoss = pSpeciesId;
	UpdateActivityTracker(false);
	lock.unlock();

	PlayerReadLock guard(PlayersMutex);

	for (Player& player : mPlayers) {
		player.LogStart(pTime);
	}
}

void Tracker::LogEnd(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
	LOG_T("{}|{}|LogEnd", pTime, pSpeciesId);

	PlayerReadLock guard(PlayersMutex);
	std::lock_guard lock(mThisMutex);

 	for (Player& player : mPlayers) {
		player.End(pTime, mActivityTracker->GetLastActiveTime());
	}

	// only add if wvw or not marked `1`
	if (mCurrentBoss > 1 || (mCurrentBoss == 1 && mIsWvW)) {
		History::instance().EmplaceBack(*this);
	}

	// reset log specifics
	mCurrentBoss = 0;

	mActivityTracker.reset();

	// remove players left while this log lasted
	for (const auto& removePlayer : mToRemovePlayers) {
		std::erase(mPlayers, removePlayer);
	}
}

void Tracker::LogNpcUpdate(uint64_t pTime, uint32_t pServerTime, uint32_t pLocalTime, uintptr_t pSpeciesId) {
	LOG_T("{}|{}|LogNpcUpdate", pTime, pSpeciesId);

	// this is the default log start, if present
	std::unique_lock lock(mThisMutex);
	mCurrentBoss = pSpeciesId;
	UpdateActivityTracker(true);
	lock.unlock();
}

void Tracker::BuffRemove(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pSrc](const auto& pPlayer) {
		return pPlayer == pSrc.id;
	});

	if (player != mPlayers.end()) {
		const auto& boons = GetBoonsFromId(pEvent->skillid);

		if (!boons) return;

		if (mActivityTracker) {
			mActivityTracker->BuffRemove(pTime, pEvent, pSrc, pDst, pSkillname, pId, pStackId);
		}

		player->RemoveBoon(boons.value(), pTime, pEvent->value, pEvent->is_buffremove, pStackId, pEvent->dst_agent, pEvent->iff);
	} else {
		const auto& boons = GetBoonsFromId(pEvent->skillid);
		if (boons) {
			if (boons.value() == Boons::Might) {
				LOG_T("unknown player buff '{}' removed. Player guess: {}", boons.value(), pSrc.name);
			} else {
				LOG_T("unknown player buff '{}' removed", boons.value());
			}
		} else {
			LOG_T("unknown player buff '{}' removed", pEvent->skillid);
		}
	}
}

void Tracker::Activation(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
	if (mActivityTracker) {
		mActivityTracker->Activation(pTime, pEvent, pSrc, pDst, pSkillname, pId);
	}
}

void Tracker::BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
	BuffApply(pTime, pEvent, pSrc, pDst, pSkillname, pId, pStackId, true);
}

void Tracker::BuffApply(uint64_t pTime, const cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId,
	bool pIsActivity) {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pDst](const auto& pPlayer) {
		return pPlayer == pDst.id;
	});

	if (player != mPlayers.end()) {
		const auto& boons = GetBoonsFromId(pEvent->skillid);

		if (!boons) return;

		if (pIsActivity && mActivityTracker) {
			mActivityTracker->BuffApply(pTime, pEvent, pSrc, pDst, pSkillname, pId, pStackId);
		}

		// is buff extension
		if (pEvent->is_offcycle) {
			player->ExtendBoon(boons.value(), pTime, pEvent->value, pStackId);
		}
		// is normal buff apply
		else {
			player->GotBoon(boons.value(), pTime, pEvent->value, pEvent->overstack_value, pStackId);
		}
	}
}

void Tracker::BuffDamage(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
	LOG_T("{}|{}:{} buff damaged {}:{}", pTime, pSrc.id, static_cast<uint64_t>(pSrc.prof), pDst.id, static_cast<uint64_t>(pDst.prof));
	if (mActivityTracker) {
		mActivityTracker->BuffDamage(pTime, pEvent, pSrc, pDst, pSkillname, pId);
	}
}

void Tracker::Strike(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId) {
	LOG_T("{}|{}:{} striked {}:{}", pTime, pSrc.id, static_cast<uint64_t>(pSrc.prof), pDst.id, static_cast<uint64_t>(pDst.prof));
	if (mActivityTracker) {
		mActivityTracker->Strike(pTime, pEvent, pSrc, pDst, pSkillname, pId);
	}
}

void Tracker::AgentAdded(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, uintptr_t pInstanceId, Prof pProfession,
                         uint32_t pElite, bool pSelf, uint16_t pTeam, uint8_t pSubgroup) {
	LOG_T("Agent '{}' addded", pAccountName);

	{
		PlayerWriteLock guard(PlayersMutex);
		std::lock_guard lock(mThisMutex);

		// if player is still there, update data.
		Player* player = GetPlayer(pId);
		if (player) {
			player->Update(pAccountName, pProfession, pSubgroup);
		} else {
			mPlayers.emplace_back(pId, pAccountName, pCharacterName, pProfession, pSelf, pSubgroup);
		}
		std::erase_if(mToRemovePlayers, [&pAccountName](const auto& pElem) { return pElem == pAccountName; });
	}

	if (pSelf) {
		// this is yourself, check your map and see active "1-logging" if it is WvW (competitive gamemode also means PvP, but there arc is disabled as a whole)
		if (auto memory = GlobalObjects::MUMBLE_MEM) {
			mIsWvW = memory->getMumbleContext()->uiState & UiStateFlags_Competitive;
		}
	}
}

void Tracker::AgentRemoved(const std::string& pAccountName, const std::string& pCharacterName, uintptr_t pId, bool pSelf) {
	LOG_T("Agent '{}' removed", pAccountName);

	PlayerWriteLock guard(PlayersMutex);
	std::lock_guard lock(mThisMutex);

	// do not remove agents while in fight.
	if (mCurrentBoss == 0) {
		std::erase(mPlayers, pAccountName);
	} else {
		mToRemovePlayers.emplace_back(pAccountName);
	}
}

void Tracker::StatReset(uint64_t pTime) {
	LOG_T("StatReset|{}", pTime);
}

void Tracker::BuffInitial(uint64_t pTime, cbtevent* pEvent, const ag& pSrc, const ag& pDst, const char* pSkillname, uint64_t pId, uint32_t pStackId) {
	LOG_T("{}|Agent '{}' buff initial '{}'", pTime, pDst.name, pSkillname);

	// BuffApply(pTime, pEvent, pSrc, pDst, pSkillname, pId, pStackId, false);

	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pDst](const auto& pPlayer) {
		return pPlayer == pDst.id;
	});

	if (player != mPlayers.end()) {
		const auto& boons = GetBoonsFromId(pEvent->skillid);

		if (!boons) return;

		player->BuffInitial(boons.value(), pTime, pEvent->value, pEvent->overstack_value, pStackId);
	}
}

void Tracker::Reward(uint64_t pTime, uintptr_t pSelfId, uintptr_t pRewardId, int32_t pRewardType) {
	if (mActivityTracker) {
		mActivityTracker->Reward(pTime);
	}
}

void Tracker::SomeActivity(uint64_t pTime) {
	for (Player& player : mPlayers) {
		player.Activity(pTime);
	}
}

void Tracker::UpdateActivityTracker(bool pNpcUpdate) {
	switch (mCurrentBoss) {
	// W1
	case std::to_underlying(TargetID::ValeGuardian):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::Gorseval):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::Sabetha):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;

	// W2
	case std::to_underlying(TargetID::Slothasor):
		// sloth seems to always be quite inprecise :(
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, mCurrentBoss);
		break;
	case std::to_underlying(TargetID::Berg):
	case std::to_underlying(TargetID::Zane):
	case std::to_underlying(TargetID::Narella):
		break;
	case std::to_underlying(TargetID::Matthias):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;

	// W3
	case std::to_underlying(TargetID::KeepConstruct):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TrashID::HauntingStatue):
		// incredible inprecise, not gonnat fix that aka. Statues is unsupported!
		break;
	case std::to_underlying(TargetID::Xera):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;

	// W4
	case std::to_underlying(TargetID::Cairn):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::MursaatOverseer): {
		IActivityTracker* oldActivityTracker = mActivityTracker.release();
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, *oldActivityTracker, mLastEventTime, mCurrentBoss);
		delete oldActivityTracker;
		break;
	}
	case std::to_underlying(TargetID::Samarog):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::Deimos):
		mActivityTracker = std::make_unique<RaidActivity>(*this, TargetID::Deimos, TrashID::Saul, TrashID::Thief, TrashID::Drunkard, TrashID::Gambler);
		break;

	// W5
	case std::to_underlying(TargetID::SoullessHorror):
	// case std::to_underlying(TargetID::Desmina): // Desmina is also SoullessHorror
	case std::to_underlying(TargetID::SoulEater):
	case std::to_underlying(TargetID::BrokenKing):
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, mCurrentBoss);
		break;
	case std::to_underlying(TargetID::Dhuum):
		mActivityTracker = std::make_unique<RaidActivity>(*this, mCurrentBoss);
		break;
	// no river support :(
	// reason: the fight actually begins before the LogNpcUpdate event.
	// aka. River and Eyes are not optimized. Isn't worth it.
	case std::to_underlying(TargetID::Desmina):
	case std::to_underlying(TargetID::EyeOfJudgement):
	case std::to_underlying(TargetID::EyeOfFate):
		break;

	// W6
	case std::to_underlying(TargetID::ConjuredAmalgamate):
	case std::to_underlying(TargetID::CARightArm):
	case std::to_underlying(TargetID::CALeftArm):
	case std::to_underlying(TargetID::ConjuredAmalgamate_CHINA):
	case std::to_underlying(TargetID::CARightArm_CHINA):
	case std::to_underlying(TargetID::CALeftArm_CHINA):
		// needs IffActivity as Default (LogNpcUpdate is after the fight actually started)
		break;
	case std::to_underlying(TargetID::Nikare):
	case std::to_underlying(TargetID::Kenut):
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, TargetID::Nikare, TargetID::Kenut);
		break;
	case std::to_underlying(TargetID::Qadim):
		// needs IffActivity as Default (LogNpcUpdate is after the fight actually started)
		break;

	// W7
	case std::to_underlying(TargetID::Sabir):
	case std::to_underlying(TargetID::Adina):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::PeerlessQadim):
		// needs IffActivity as Default (LogNpcUpdate is after the fight actually started)
		break;

	// EoD Strikes
	case std::to_underlying(TargetID::MaiTrinStrike):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::Ankka):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::MinisterLi):
	case std::to_underlying(TargetID::MinisterLiCM):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		mActivityTracker->SetRewardActive(false);
		break;
	case std::to_underlying(TargetID::PrototypeVermilion):
	case std::to_underlying(TargetID::PrototypeArsenite):
	case std::to_underlying(TargetID::PrototypeIndigo):
	case std::to_underlying(TargetID::PrototypeVermilionCM):
	case std::to_underlying(TargetID::PrototypeArseniteCM):
	case std::to_underlying(TargetID::PrototypeIndigoCM):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		break;
	case std::to_underlying(TargetID::GadgetTheDragonVoid1):
	case std::to_underlying(TargetID::GadgetTheDragonVoid2):
		// mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		mActivityTracker->SetRewardActive(false);
		break;

	// Fractals
	// 98CM
	case std::to_underlying(TargetID::MAMA):
		// mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		// dynamic_cast<TestActivity&>(*mActivityTracker).LogNpcUpdate(mLastEventTime, TargetID::MAMA, TrashID::GreenKnight, TrashID::RedKnight, TrashID::BlueKnight);
		// mActivityTracker->SetRewardActive(false);
		break;
	case std::to_underlying(TargetID::Siax):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		mActivityTracker->SetRewardActive(false);
		break;
	case std::to_underlying(TargetID::Ensolyss):
		// mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		mActivityTracker->SetRewardActive(false);
		break;
	// 99CM
	case std::to_underlying(TargetID::Skorvald):
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, TargetID::Skorvald, TrashID::FluxAnomaly1, TrashID::FluxAnomaly2, TrashID::FluxAnomaly3, TrashID::FluxAnomaly4);
		mActivityTracker->SetRewardActive(false);
		break;
	case std::to_underlying(TargetID::Artsariiv):
		mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		mActivityTracker->SetRewardActive(false);
		break;
	case std::to_underlying(TargetID::Arkk):
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, TargetID::Arkk, TrashID::Archdiviner, TrashID::BrazenGladiator);
		mActivityTracker->SetRewardActive(false);
		break;
	// 100CM
	case std::to_underlying(TargetID::AiKeeperOfThePeak):
		mActivityTracker = std::make_unique<RaidNpcTracker>(*this, mLastEventTime, TargetID::AiKeeperOfThePeak, TrashID::SorrowDemon5, TrashID::FearDemon, TrashID::GuiltDemon, TrashID::EnrageWaterSprite);
		mActivityTracker->SetRewardActive(false);
		break;

	default:
		if (pNpcUpdate) {
			mActivityTracker = std::make_unique<IffNpcTracker>(*this, mLastEventTime);
		} else {
			mActivityTracker = std::make_unique<IffActivity>(*this);
			// mActivityTracker = std::make_unique<TestActivity>(*this, mLastEventTime);
		}
	}
}

Player* Tracker::GetPlayer(uintptr_t pId) {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

Player* Tracker::GetPlayer(const std::string& pId) {
	const auto& player = std::ranges::find_if(mPlayers, [pId](const Player& pPlayer){ return pPlayer == pId; });

	if (player != mPlayers.end()) {
		return &*player;
	}
	return nullptr;
}

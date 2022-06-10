#include "Tracker.h"

#include "GlobalObjects.h"

#include <ranges>

void Tracker::Event(cbtevent* pEvent, ag* pSrc, ag* pDst, const char* pSkillname, uint64_t pId) {
	if (pEvent) {
		// times are saved as double in seconds
		mCurrentTime = static_cast<double>(pEvent->time) / 1000;

		// buff applied event
		if (pEvent->buff && pEvent->buff_dmg == 0 && pEvent->is_buffremove == 0) {
			BuffAppliedEvent(pEvent, pSrc, pDst);
		}
		// buff removed event
		if (pEvent->is_buffremove && pEvent->buff && pEvent->is_statechange == 0) {
			BuffRemoveEvent(pEvent, pSrc);
		}
		// state change event
		else if (cbtstatechange statechange = pEvent->is_statechange) {
			PlayerReadLock guard(PlayersMutex);

			if (statechange == CBTS_ENTERCOMBAT) {
				const auto& player = std::ranges::find_if(mPlayers, [pSrc](const auto& pPlayer) {
					return pPlayer == pSrc->id;
				});

				if (player != mPlayers.end()) {
					player->EnterCombat(pEvent);
				}
			} else if (statechange == CBTS_EXITCOMBAT) {
				const auto& player = std::ranges::find_if(mPlayers, [pSrc](const auto& pPlayer) {
					return pPlayer == pSrc->id;
				});

				if (player != mPlayers.end()) {
					player->ExitCombat(pEvent);
				}
			}
		}
	} else {
		if (pSrc) {
			/* notify tracking change */
			if (!pSrc->elite) {
				/* add */
				if (pSrc->prof) {
					if (pDst && pDst->name) {
						AddPlayer(pSrc, pDst);

						if (pDst->self) {
							// this is yourself, check your map and see active "1-logging" if it is WvW
							if (auto memory = GlobalObjects::MUMBLE_MEM) {
								constexpr std::array<uint32_t, 4> mapIDs{38, 95, 96, 1099};
								uint32_t mapId = memory->getMumbleContext()->mapId;
								const auto& mapIdIt = std::find(mapIDs.begin(), mapIDs.end(), mapId);
								mIsWvW = mapIdIt != mapIDs.end();
							}
						}
					}
				}
				/* remove */
				else {
					RemovePlayer(pDst);
				}
			}
		}
	}
}

double Tracker::GetTime() const {
	return mCurrentTime;
}

const std::vector<Player>& Tracker::GetAllPlayer() {
	return mPlayers;
}

void Tracker::AddPlayer(ag* pSrc, ag* pDst) {
	PlayerWriteLock guard(PlayersMutex);

	std::string accountName = pDst->name;

	if (accountName.at(0) == ':') {
		accountName.erase(0, 1);
	}

	mPlayers.emplace_back(pSrc->id, accountName, pSrc->name, pDst->prof, pDst->self, static_cast<uint8_t>(pDst->team));
}

void Tracker::RemovePlayer(ag* pDst) {
	PlayerWriteLock guard(PlayersMutex);

	std::erase(mPlayers, pDst->name);
}

void Tracker::BuffAppliedEvent(cbtevent* pEvent, ag* pSrc, ag* pDst) {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pDst](const auto& pPlayer) {
		return pPlayer == pDst->id;
	});

	if (player != mPlayers.end()) {
		const auto& boons = GetBoonsFromId(pEvent->skillid);

		if (!boons) return;

		int32_t pDuration;
		//is normal buff apply
		if (!pEvent->is_offcycle) {
			pDuration = pEvent->value - pEvent->overstack_value;
		}
		//is buff extension
		else
		{
			pDuration = pEvent->value;
		}

		player->GotBoon(boons.value(), static_cast<double>(pEvent->time) / 1000, pDuration);
	}
}

void Tracker::BuffRemoveEvent(cbtevent* pEvent, ag* pSrc) {
	PlayerReadLock guard(PlayersMutex);

	const auto& player = std::ranges::find_if(mPlayers, [pSrc](const auto& pPlayer) {
		return pPlayer == pSrc->id;
	});

	if (player != mPlayers.end()) {
		const auto& boons = GetBoonsFromId(pEvent->skillid);

		if (!boons) return;

		int32_t pDuration;
		//is normal buff apply
		if (!pEvent->is_offcycle) {
			pDuration = pEvent->value - pEvent->overstack_value;
		}
		//is buff extension
		else
		{
			pDuration = pEvent->value;
		}

		player->RemoveBoon(boons.value(), static_cast<double>(pEvent->time) / 1000, pDuration, pEvent->result, pEvent->is_buffremove);
	}
}

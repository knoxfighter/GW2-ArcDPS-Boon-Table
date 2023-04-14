#define _CRT_SECURE_NO_WARNINGS

#include "CombatMock.h"

#include "Xevtc.h"

#include <nlohmann/json.hpp>

#include <atomic>
#include <cassert>
#include <Windows.h>

#ifndef LOG
#define LOG
#endif

using json = nlohmann::json;

void CombatMock::AddAgent(const char* pAgentName, const char* pAccountName, Prof pProfession, uint32_t pElite, uint8_t pSubgroup, uint64_t pMasterUniqueId)
{
	Agent& newAgent = myAgents.emplace_back();
	newAgent.AgentName = pAgentName;
	newAgent.AccountName = pAccountName;
	newAgent.Profession = pProfession;
	newAgent.Elite = pElite;
	newAgent.Subgroup = pSubgroup;
	newAgent.MasterUniqueId = pMasterUniqueId;

	newAgent.UniqueId = myNextUniqueId++;
	newAgent.InstanceId = 0;
}

void CombatMock::AddEvent(CombatEventType pType, uint64_t pTime, uint64_t pSourceAgentUniqueId, uint64_t pDestinationAgentUniqueId, uint32_t pSkillId, int32_t pValue, bool pIsBuff)
{
	CombatEvent& newEvent = myEvents.emplace_back();
	newEvent.Type = pType;
	newEvent.Time = pTime;
	newEvent.SourceAgentUniqueId = pSourceAgentUniqueId;
	newEvent.DestinationAgentUniqueId = pDestinationAgentUniqueId;
	newEvent.SkillId = pSkillId;
	newEvent.Value = pValue;
	newEvent.IsBuff = pIsBuff;
}

CombatMock::CombatMock(const arcdps_exports* pCallbacks)
	: myCallbacks(pCallbacks)
{
}

uint32_t CombatMock::SaveToFile(const char* pFilePath)
{
	json jsonAgents = json::array();
	for (const auto& curAgent : myAgents)
	{
		jsonAgents.push_back(
		{
			{"AgentName", curAgent.AgentName},
			{"AccountName", curAgent.AccountName},
			{"Profession", curAgent.Profession},
			{"Elite", curAgent.Elite},
			{"Subgroup", curAgent.Subgroup},
			{"MasterInstanceId", curAgent.MasterUniqueId},
			{"UniqueId", curAgent.UniqueId}
		});
	}

	json jsonEvents = json::array();
	for (const auto& curEvent : myEvents)
	{
		jsonEvents.push_back(
		{
			{"Type", curEvent.Type},
			{"Time", curEvent.Time},
			{"SourceAgentUniqueId", curEvent.SourceAgentUniqueId},
			{"DestinationAgentUniqueId", curEvent.DestinationAgentUniqueId},
			{"SkillId", curEvent.SkillId},
			{"Value", curEvent.Value},
			{"IsBuff", curEvent.IsBuff}
		});
	}

	json jsonObject =
	{
		{"version", 1},

		{"myNextUniqueId", myNextUniqueId},
		{"mySelfId", mySelfId},

		{"myAgents", jsonAgents},
		{"myEvents", jsonEvents}
	};

	std::string serialized = jsonObject.dump();
	FILE* file = fopen(pFilePath, "w");
	if (file == nullptr)
	{
		printf("SaveToFile: Opening '%s' failed - %u\n", pFilePath, errno);
		return errno;
	}

	size_t written = fwrite(serialized.c_str(), sizeof(char), serialized.size(), file);
	if (written != serialized.size())
	{
		printf("SaveToFile: Writing to '%s' failed - %zu %u\n", pFilePath, written, errno);
		return errno;
	}

	if (fclose(file) == EOF)
	{
		printf("SaveToFile: Closing '%s' failed - %u\n", pFilePath, errno);
		return errno;
	}
	
	printf("SaveToFile: Success\n");
	return 0;
}

uint32_t CombatMock::LoadFromFile(const char* pFilePath)
{
	std::string buffer;
	buffer.reserve(128 * 1024);

	FILE* file = fopen(pFilePath, "r");
	if (file == nullptr)
	{
		printf("LoadFromFile: Opening '%s' failed - %u\n", pFilePath, errno);
		return errno;
	}

	size_t read = fread(buffer.data(), sizeof(char), 128 * 1024 - 1, file);
	if (read == 128 * 1024 - 1)
	{
		printf("LoadFromFile: Reading '%s' failed - insufficient buffer\n", pFilePath);
		return ENOBUFS;
	}
	else if (ferror(file) != 0)
	{
		printf("LoadFromFile: Reading '%s' failed - %u\n", pFilePath, errno);
		return errno;
	}

	if (fclose(file) == EOF)
	{
		printf("LoadFromFile: Closing '%s' failed - %u\n", pFilePath, errno);
		return errno;
	}

	buffer.data()[read] = '\0';
	printf("LoadFromFile: Parsing %s\n", buffer.data());

	json jsonObject = json::parse(buffer.data());

	for (auto& [key, value] : jsonObject.items())
	{
		if ((key == "myNextUniqueId") && (value.is_number_unsigned() == true))
		{
			myNextUniqueId = value.get<uint64_t>();
		}
		else if ((key == "mySelfId") && (value.is_number_unsigned() == true))
		{
			mySelfId = value.get<uint64_t>();
		}
		else if ((key == "myAgents") && (value.is_array() == true))
		{
			myAgents.clear();
			for (auto& curAgent : value.get<json::array_t>())
			{
				Agent& newAgent = myAgents.emplace_back();
				for (auto& [key2, value2] : curAgent.items())
				{
					if ((key2 == "AgentName") && (value2.is_string() == true))
					{
						newAgent.AgentName = value2.get<std::string>();
					}
					else if ((key2 == "AccountName") && (value2.is_string() == true))
					{
						newAgent.AccountName = value2.get<std::string>();
					}
					else if ((key2 == "Profession") && (value2.is_number_unsigned() == true))
					{
						newAgent.Profession = static_cast<Prof>(value2.get<uint32_t>());
					}
					else if ((key2 == "Elite") && (value2.is_number_unsigned() == true))
					{
						newAgent.Elite = value2.get<uint32_t>();
					}
					else if ((key2 == "Subgroup") && (value2.is_number_unsigned() == true))
					{
						newAgent.Subgroup = value2.get<uint8_t>();
					}
					else if ((key2 == "MasterUniqueId") && (value2.is_number_unsigned() == true))
					{
						newAgent.MasterUniqueId = value2.get<uint64_t>();
					}
					else if ((key2 == "UniqueId") && (value2.is_number_unsigned() == true))
					{
						newAgent.UniqueId = value2.get<uint64_t>();
					}
				}
			}
		}
		else if ((key == "myEvents") && (value.is_array() == true))
		{
			myEvents.clear();
			for (auto& curEvent : value.get<json::array_t>())
			{
				CombatEvent& newEvent = myEvents.emplace_back();
				for (auto& [key2, value2] : curEvent.items())
				{
					if ((key2 == "Type") && (value2.is_number_unsigned() == true))
					{
						newEvent.Type = value2.get<CombatEventType>();
					}
					else if ((key2 == "Time") && (value2.is_number_unsigned() == true))
					{
						newEvent.Time = value2.get<uint64_t>();
					}
					else if ((key2 == "SourceAgentUniqueId") && (value2.is_number_unsigned() == true))
					{
						newEvent.SourceAgentUniqueId = value2.get<uint64_t>();
					}
					else if ((key2 == "DestinationAgentUniqueId") && (value2.is_number_unsigned() == true))
					{
						newEvent.DestinationAgentUniqueId = value2.get<uint64_t>();
					}
					else if ((key2 == "SkillId") && (value2.is_number_unsigned() == true))
					{
						newEvent.SkillId = value2.get<uint32_t>();
					}
					else if ((key2 == "Value") && (value2.is_number() == true))
					{
						newEvent.Value = value2.get<int32_t>();
					}
					else if ((key2 == "IsBuff") && (value2.is_boolean() == true))
					{
						newEvent.IsBuff = value2.get<bool>();
					}
				}
			}
		}
	}

	printf("LoadFromFile: Success\n");
	return 0;
}

const CombatMock::Agent* CombatMock::GetAgent(uint64_t pAgentUniqueId)
{
	for (const Agent& agent : myAgents)
	{
		if (agent.UniqueId == pAgentUniqueId)
		{
			return &agent;
		}
	}

	return nullptr;
}

void CombatMock::FillAgent(const Agent* pAgent, uint64_t pSelfId, ag& pResult)
{
	if (pAgent == nullptr)
	{
		return;
	}

	pResult.name = pAgent->AgentName.c_str();
	pResult.id = pAgent->UniqueId;
	pResult.prof = pAgent->Profession;
	pResult.elite = pAgent->Elite;
	pResult.self = (pAgent->UniqueId == mySelfId) ? 1 : 0;
	pResult.team = 189; // What does this value even mean?
}

void CombatMock::FillAgentEvent(const Agent& pAgent, uint64_t pSelfId, ag& pSource, ag& pDestination)
{
	pSource.name = pAgent.AgentName.c_str();
	pSource.id = pAgent.UniqueId;
	pSource.self = 0; // I don't think this is used at all?
	pSource.team = 189; // What does this value even mean?

	pDestination.name = pAgent.AccountName.c_str();
	pDestination.id = pAgent.InstanceId;
	pDestination.prof = pAgent.Profession;
	pDestination.elite = pAgent.Elite;
	pDestination.self = (pAgent.UniqueId == mySelfId) ? 1 : 0;
	pDestination.team = pAgent.Subgroup;
}

void CombatMock::Execute()
{
	uint64_t idCounter = 1;

	// Send agent registration events
	for (Agent& agent : myAgents)
	{
		// Generate InstanceId for this agent
		agent.InstanceId = myNextInstanceId++;

		ag source = {};
		ag destination = {};
		FillAgentEvent(agent, mySelfId, source, destination);

		source.prof = static_cast<Prof>(1); // indicates this is an agent registration event
		source.elite = 0; // indicates this is an agent registration event

		if (myCallbacks->combat != nullptr)
		{
			myCallbacks->combat(nullptr, &source, &destination, nullptr, idCounter, 1);
		}
		if ((myCallbacks->combat_local != nullptr) && (mySelfId != 0) && (agent.UniqueId == mySelfId))
		{
			myCallbacks->combat_local(nullptr, &source, &destination, nullptr, idCounter, 1);
		}
		idCounter++;
	}

	for (const CombatEvent& eventIter : myEvents)
	{
		ag source = {};
		ag destination = {};
		cbtevent combatEvent = {};

		const Agent* sourceAgent = nullptr;
		const Agent* sourceAgentMaster = nullptr;

		if (eventIter.SourceAgentUniqueId != 0)
		{
			sourceAgent = GetAgent(eventIter.SourceAgentUniqueId);
			assert(sourceAgent != nullptr);

			if (sourceAgent->MasterUniqueId != 0)
			{
				sourceAgentMaster = GetAgent(sourceAgent->MasterUniqueId);
				assert(sourceAgentMaster != nullptr);
			}
		}

		switch (eventIter.Type)
		{
		case CombatEventType::Direct:
		{
			const Agent* destinationAgent = nullptr;
			const Agent* destinationAgentMaster = nullptr;
			if (eventIter.DestinationAgentUniqueId != 0)
			{
				destinationAgent = GetAgent(eventIter.DestinationAgentUniqueId);
				assert(destinationAgent != nullptr);

				if (destinationAgent->MasterUniqueId != 0)
				{
					destinationAgentMaster = GetAgent(destinationAgent->MasterUniqueId);
					assert(destinationAgentMaster != nullptr);
				}
			}

			FillAgent(sourceAgent, mySelfId, source);
			FillAgent(destinationAgent, mySelfId, destination);

			combatEvent.time = eventIter.Time;

			if (sourceAgent != nullptr)
			{
				combatEvent.src_agent = sourceAgent->UniqueId;
				combatEvent.src_instid = sourceAgent->InstanceId;
			}
			else
			{
				combatEvent.src_agent = 0;
				combatEvent.src_instid = 0;
			}

			if (destinationAgent != nullptr)
			{
				combatEvent.dst_agent = destinationAgent->UniqueId;
				combatEvent.dst_instid = destinationAgent->InstanceId;
			}
			else
			{
				combatEvent.dst_agent = 0;
				combatEvent.dst_instid = 0;
			}

			if (sourceAgentMaster != nullptr)
			{
				combatEvent.src_master_instid = sourceAgentMaster->InstanceId;
			}
			else
			{
				combatEvent.src_master_instid = 0;
			}

			if (destinationAgentMaster != nullptr)
			{
				combatEvent.dst_master_instid = destinationAgentMaster->InstanceId;
			}
			else
			{
				combatEvent.dst_master_instid = 0;
			}

			if (eventIter.IsBuff == false)
			{
				combatEvent.buff = 0;
				combatEvent.value = eventIter.Value;
				combatEvent.buff_dmg = 0;
				combatEvent.result = CBTR_NORMAL;
				combatEvent.is_offcycle = 0; // Target is not downed
				combatEvent.overstack_value = 0; // Strike did not hit shield
			}
			else
			{
				combatEvent.buff = 1;
				combatEvent.value = 0; // It's a damage event and not a buff application event
				combatEvent.buff_dmg = eventIter.Value;
				combatEvent.result = 0; // Target is not invulnerable
				combatEvent.is_offcycle = 0; // It's a tick
				combatEvent.overstack_value = 0; // Does this have any meaning here?
			}

			combatEvent.skillid = eventIter.SkillId;

			if (eventIter.Value > 0)
			{
				combatEvent.iff = IFF_FOE;
			}
			else
			{
				combatEvent.iff = IFF_FRIEND;
			}

			combatEvent.is_activation = ACTV_NONE;
			combatEvent.is_buffremove = CBTB_NONE;
			combatEvent.is_ninety = 1; // simulate as if everyone is full hp
			combatEvent.is_fifty = 0; // simulate as if everyone is full hp
			combatEvent.is_moving = 0;
			combatEvent.is_statechange = CBTS_NONE;
			combatEvent.is_flanking = 0;
			combatEvent.is_shields = 0;
			combatEvent.pad61 = 0;
			combatEvent.pad62 = 0;
			combatEvent.pad63 = 0;
			combatEvent.pad64 = 0;

			// Send event
			char skillNameBuffer[128];
			snprintf(skillNameBuffer, sizeof(skillNameBuffer), "Skill %u", eventIter.SkillId);
			auto [skillIter, _inserted] = mySkillNames.try_emplace(eventIter.SkillId, skillNameBuffer);

			if (myCallbacks->combat != nullptr && (eventIter.Value > 0)) // don't send heal events to area callback
			{
				myCallbacks->combat(&combatEvent, &source, &destination, skillIter->second.c_str(), idCounter, 1);
			}
			if ((myCallbacks->combat_local != nullptr) && (mySelfId != 0)
				&& ((sourceAgent != nullptr && sourceAgent->UniqueId == mySelfId)
					|| (destinationAgent != nullptr && destinationAgent->UniqueId == mySelfId)
					|| (sourceAgentMaster != nullptr && sourceAgentMaster->UniqueId == mySelfId)
					|| (destinationAgentMaster != nullptr && destinationAgentMaster->UniqueId == mySelfId)))
			{
				// For some reason values in local events are flipped in arcdps
				combatEvent.value = combatEvent.value * -1;
				combatEvent.buff_dmg = combatEvent.buff_dmg * -1;

				myCallbacks->combat_local(&combatEvent, &source, &destination, skillIter->second.c_str(), idCounter, 1);
			}
			idCounter++;

			break;
		}
		case CombatEventType::EnterCombat:
		case CombatEventType::ExitCombat:
		{
			assert(sourceAgent != nullptr);
			FillAgent(sourceAgent, mySelfId, source);

			combatEvent.time = eventIter.Time;
			combatEvent.src_agent = sourceAgent->UniqueId;
			combatEvent.src_instid = sourceAgent->InstanceId;
			combatEvent.dst_agent = sourceAgent->Subgroup;

			if (sourceAgentMaster != nullptr)
			{
				combatEvent.src_master_instid = sourceAgentMaster->InstanceId;
			}

			if (eventIter.Type == CombatEventType::EnterCombat)
			{
				combatEvent.is_statechange = CBTS_ENTERCOMBAT;
			}
			else
			{
				assert(eventIter.Type == CombatEventType::ExitCombat);
				combatEvent.is_statechange = CBTS_EXITCOMBAT;
			}

			// The rest of the event is filled with 0 above

			// Send event
			if (myCallbacks->combat != nullptr) // don't send heal events to area callback
			{
				myCallbacks->combat(&combatEvent, &source, nullptr, nullptr, idCounter, 1);
			}
			if ((myCallbacks->combat_local != nullptr) && (mySelfId != 0)
				&& ((sourceAgent != nullptr && sourceAgent->UniqueId == mySelfId)
					|| (sourceAgentMaster != nullptr && sourceAgentMaster->UniqueId == mySelfId)))
			{
				myCallbacks->combat_local(&combatEvent, &source, nullptr, nullptr, idCounter, 1);
			}
			idCounter++;

			break;
		}
		default:
			break;
		}
	}
}

bool IsSelfAgentDeregister(XevtcEvent& pEvent)
{
	return pEvent.ev.present == false && pEvent.source_ag.elite == 0 && pEvent.source_ag.prof == 0 && pEvent.destination_ag.self != 0;
}

void ExecuteXevtcEvent(const XevtcEvent& pEvent, const std::vector<std::string>& pStrings, const arcdps_exports& pCallbacks)
{
	ag source;
	ag destination;
	cbtevent ev;

	ag* source_arg = nullptr;
	ag* destination_arg = nullptr;
	cbtevent* ev_arg = nullptr;
	const char* skillname = nullptr;

	if (pEvent.ev.present == true)
	{
		ev = *static_cast<const cbtevent*>(&pEvent.ev);
		ev_arg = &ev;
	}

	if (pEvent.source_ag.present == true)
	{
		source.id = pEvent.source_ag.id;
		source.prof = pEvent.source_ag.prof;
		source.elite = pEvent.source_ag.elite;
		source.self = pEvent.source_ag.self;
		if (pEvent.source_ag.name.Index != UINT32_MAX)
		{
			source.name = pStrings[pEvent.source_ag.name.Index - 1].c_str();
		}
		else
		{
			source.name = nullptr;
		}
		source.team = pEvent.source_ag.team;

		source_arg = &source;
	}

	if (pEvent.destination_ag.present == true)
	{
		destination.id = pEvent.destination_ag.id;
		destination.prof = pEvent.destination_ag.prof;
		destination.elite = pEvent.destination_ag.elite;
		destination.self = pEvent.destination_ag.self;
		if (pEvent.destination_ag.name.Index != UINT32_MAX)
		{
			destination.name = pStrings[pEvent.destination_ag.name.Index - 1].c_str();
		}
		else
		{
			destination.name = nullptr;
		}
		destination.team = pEvent.destination_ag.team;

		destination_arg = &destination;
	}

	if (pEvent.skillname.Index != UINT32_MAX)
	{
		skillname = pStrings[pEvent.skillname.Index - 1].c_str();
	}

	switch (pEvent.collector_source)
	{
	case XevtcEventSource::Area:
		if (pCallbacks.combat != nullptr)
		{
			pCallbacks.combat(ev_arg, source_arg, destination_arg, skillname, pEvent.id, pEvent.revision);
		}
		break;
	case XevtcEventSource::Local:
		if (pCallbacks.combat_local != nullptr)
		{
			pCallbacks.combat_local(ev_arg, source_arg, destination_arg, skillname, pEvent.id, pEvent.revision);
		}
		break;
	default:
		LOG("Invalid event source %u", pEvent.collector_source);
		break;
	}
}

struct CallbackWorkerArguments
{
	const XevtcEvent* pEvents;
	std::atomic_bool* pEventsSent;
	uint32_t pEventCount;
	uint32_t pParallelCount;
	const std::vector<uint32_t>* pJunctions;
	std::atomic_uint32_t* pIndexSeq;

	const std::vector<std::string>* pStrings;
	const arcdps_exports* pCallbacks;
};

DWORD WINAPI CallbackWorker(LPVOID pParam)
{
	CallbackWorkerArguments* args = reinterpret_cast<CallbackWorkerArguments*>(pParam);
	//LOG(">> %x >> %p %p %u %u %p %p %p %p", GetCurrentThreadId(), args->pEvents, args->pEventsSent, args->pEventCount, args->pParallelCount, args->pJunctions, args->pIndexSeq, args->pStrings, args->pCallbacks);

	uint32_t notifiedIndex = 0;
	uint32_t currentJunction = 0;
	uint32_t executedCount = 0;
	while (true)
	{
		uint32_t index = args->pIndexSeq->fetch_add(1, std::memory_order_relaxed);
		if (index >= args->pEventCount)
		{
			break;
		}

		bool pastJunction = false;
		uint32_t junctionIndex = UINT32_MAX;
		if (currentJunction < args->pJunctions->size())
		{
			junctionIndex = (*args->pJunctions)[currentJunction];
		}

		if (index > junctionIndex)
		{
			pastJunction = true;
			currentJunction++;
		}

		while (true)
		{
			while (true)
			{
				assert(notifiedIndex < args->pEventCount); // Otherwise, someone would have to have notified our index
				if (args->pEventsSent[notifiedIndex].load(std::memory_order_relaxed) == false)
				{
					break;
				}

				notifiedIndex++;
			}

			if (index >= (notifiedIndex + args->pParallelCount))
			{
				//LOG("Thread %x waiting with index %u due to parallelism - %u %u", GetCurrentThreadId(), index, notifiedIndex, args->pParallelCount);
				Sleep(0);
				continue;
			}

			if (pastJunction == true && notifiedIndex <= junctionIndex)
			{
				//LOG("Thread %x waiting with index %u at junction - %u %u", GetCurrentThreadId(), index, notifiedIndex, junctionIndex);
				Sleep(0);
				continue;
			}

			break;
		}

		//LOG("Thread %x sending %u", GetCurrentThreadId(), index);
		ExecuteXevtcEvent(args->pEvents[index], *args->pStrings, *args->pCallbacks);

		args->pEventsSent[index].store(true, std::memory_order_relaxed);
		executedCount++;
	}

	LOG("Thread %x exiting - %u events executed", GetCurrentThreadId(), executedCount);
	return 0;
}


uint32_t CombatMock::ExecuteFromXevtc(const char* pFilePath, uint32_t pMaxParallelEventCount, uint32_t pMaxFuzzWidth)
{
	LOG("Executing '%s' - pMaxParallelEventCount=%u, pMaxFuzzWidth=%u", pFilePath, pMaxParallelEventCount, pMaxFuzzWidth);

	if (pMaxFuzzWidth > 0)
	{
		uint32_t seed = timeGetTime();
		LOG("Using seed %u", seed);
		srand(seed);
	}

	std::string buffer;
	buffer.reserve(128 * 1024);

	FILE* file = fopen(pFilePath, "rb");
	if (file == nullptr)
	{
		LOG("Opening '%s' failed - %u", pFilePath, errno);
		return errno;
	}

	XevtcHeader header;
	size_t read = fread(&header, sizeof(header), 1, file);
	if (ferror(file) != 0)
	{
		LOG("Reading header from '%s' failed - %u", pFilePath, errno);
		return errno;
	}
	else if (read != 1)
	{
		LOG("Reading header from '%s' failed - file is too short (read %zu)", pFilePath, read);
		return UINT32_MAX;
	}

	mXevtcStrings.clear();
	mXevtcStrings.reserve(header.StringCount);
	for (uint32_t i = 0; i < header.StringCount; i++)
	{
		uint16_t size;
		read = fread(&size, sizeof(size), 1, file);
		if (ferror(file) != 0)
		{
			LOG("Reading string header %u from '%s' failed - %u", i, pFilePath, errno);
			return errno;
		}
		else if (read != 1)
		{
			LOG("Reading string header %u from '%s' failed - file is too short (read %zu)", i, pFilePath, read);
			return UINT32_MAX;
		}

		std::string& newString = mXevtcStrings.emplace_back();
		newString.resize(size);

		if (size != 0) // null strings are allowed in the xevtc
		{
			read = fread(newString.data(), size, 1, file);
			if (ferror(file) != 0)
			{
				LOG("Reading string data %u (size %hu) from '%s' failed - %u", i, size, pFilePath, errno);
				return errno;
			}
			else if (read != 1)
			{
				LOG("Reading string data %u (size %hu) from '%s' failed - file is too short (read %zu)", i, size, pFilePath, read);
				return UINT32_MAX;
			}
		}

		LOG("Parsed string %u %hu %s", i, size, newString.c_str());
	}

	auto eventsVector = std::make_unique<XevtcEvent[]>(header.EventCount);
	read = fread(eventsVector.get(), sizeof(XevtcEvent), header.EventCount, file);
	if (ferror(file) != 0)
	{
		LOG("Reading events (size %hu) from '%s' failed - %u", header.EventCount, pFilePath, errno);
		return errno;
	}
	else if (read != header.EventCount)
	{
		LOG("Reading events (size %hu) from '%s' failed - file is too short (read %zu)", header.EventCount, pFilePath, read);
		return UINT32_MAX;
	}

	if (fclose(file) == EOF)
	{
		LOG("Closing '%s' failed - %u", pFilePath, errno);
		return errno;
	}

	auto queuedEvents = std::make_unique<bool[]>(header.EventCount);
	memset(queuedEvents.get(), 0x00, header.EventCount * sizeof(bool));
	uint32_t queuedEventCount = 0;

	bool sentFirstEvent = false;
	
	auto eventQueue = std::make_unique<XevtcEvent[]>(header.EventCount);
	std::vector<uint32_t> junctions;

	uint32_t globalIndex = 0;
	while (globalIndex < header.EventCount)
	{
		// Use UINT64_MAX as a marker to determine if events have been sent
		if (queuedEvents[globalIndex] == true)
		{
			globalIndex++;
			continue;
		}

		uint32_t fuzzSize = 0;
		if (pMaxFuzzWidth > 0 && sentFirstEvent == true)
		{
			fuzzSize = rand() % (pMaxFuzzWidth + 1);
		}

		uint32_t localIndex = globalIndex;
		while ((localIndex + 1) < header.EventCount && localIndex < (globalIndex + fuzzSize))
		{
			if (IsSelfAgentDeregister(eventsVector[localIndex]) == true)
			{
				break;
			}
			localIndex++;
		}

		while (queuedEvents[localIndex] == true)
		{
			assert(localIndex > globalIndex);
			localIndex--;
		}
		assert(queuedEvents[localIndex] == false);

		memcpy(&eventQueue[queuedEventCount], &eventsVector[localIndex], sizeof(eventQueue[queuedEventCount]));
		if (IsSelfAgentDeregister(eventsVector[localIndex]) == true)
		{
			junctions.push_back(queuedEventCount);
		}
		else if (eventsVector[localIndex].id != 0 && sentFirstEvent == false)
		{
			junctions.push_back(queuedEventCount);
			sentFirstEvent = true;
		}

		queuedEvents[localIndex] = true;
		queuedEventCount += 1;
	}

	if (pMaxParallelEventCount > 0)
	{
		LOG("Starting %u threads", pMaxParallelEventCount);

		auto eventsSent = std::make_unique<std::atomic_bool[]>(header.EventCount);
		for (uint32_t i = 0; i < header.EventCount; i++)
		{
			eventsSent[i].store(false, std::memory_order_relaxed);
		}
		std::atomic_uint32_t indexSeq = 0;

		CallbackWorkerArguments threadArguments;
		threadArguments.pEvents = eventQueue.get();
		threadArguments.pEventsSent = eventsSent.get();
		threadArguments.pEventCount = header.EventCount;
		threadArguments.pParallelCount = pMaxParallelEventCount;
		threadArguments.pJunctions = &junctions;
		threadArguments.pIndexSeq = &indexSeq;

		threadArguments.pStrings = &mXevtcStrings;
		threadArguments.pCallbacks = myCallbacks;

		std::vector<HANDLE> threadHandles;
		for (uint32_t i = 0; i < pMaxParallelEventCount; i++)
		{
			HANDLE handle = CreateThread(nullptr, 0, CallbackWorker, &threadArguments, 0, nullptr);
			threadHandles.emplace_back(handle);
		}

		LOG("Waiting for %zu threads to finish", threadHandles.size());
		for (uint32_t i = 0; i < threadHandles.size(); i++)
		{
			DWORD result = WaitForSingleObject(threadHandles[i], UINT32_MAX);
			if (result != 0)
			{
				LOG("Waiting for thread %u reslted in %u GetLastError %u", i, result, GetLastError());
			}
		}
		LOG("Waiting for threads finished");

		for (uint32_t i = 0; i < pMaxParallelEventCount; i++)
		{
			CloseHandle(threadHandles[i]);
		}
		LOG("Threads closed");
	}
	else
	{
		LOG("Started sending events synchronously");
		for (uint32_t i = 0; i < header.EventCount; i++)
		{
			ExecuteXevtcEvent(eventQueue[i], mXevtcStrings, *myCallbacks);
		}
		LOG("Done sending events synchronously");
	}

	LOG("Simulated %u events from %s", header.EventCount, pFilePath);
	return 0;
}

// void CombatMock::DisplayWindow()
// {
// 	DisplayAgents();
// 	DisplayEvents();
// 	DisplayAddAgent();
// 	DisplayAddEvent();
// 	DisplayActions();
//
// 	if (showLog == true)
// 	{
// 		DisplayLog();
// 	}
// }

void CombatMock::e3LogLine(const std::string& line) {
	if (e3_log.size() > linesToKeep)
	{
		e3_log.pop_front();
	}
	e3_log.push_back(line);
}

void CombatMock::e8LogLine(const std::string& line) {
	if (e8_log.size() > linesToKeep)
	{
		e8_log.pop_front();
	}
	e8_log.push_back(line);
}

// void CombatMock::DisplayAgents()
// {
// 	ImGui::SetNextWindowSize(ImVec2(900, -1), ImGuiCond_FirstUseEver);
// 	if (ImGui::Begin("Agents", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar) == true)
// 	{
// 		ImGui::BeginTable("##AgentTable", 10, ImGuiTableFlags_SizingStretchProp);
//
// 		for (const auto& label : { "Agent Name", "Account Name", "Profession", "Elite", "Subgroup", "Master Instance Id", "Unique Id", "Instance Id", "Is Self" })
// 		{
// 			ImGui::TableNextColumn();
// 			ImGui::TextUnformatted(label);
// 		}
// 		ImGui::TableNextColumn(); // empty label in remove column
//
// 		int i = 0;
// 		auto iter = myAgents.begin();
// 		while (iter != myAgents.end())
// 		{
// 			ImGui::PushID(i); // Needed for button to work
//
// 			ImGui::TableNextColumn();
// 			ImGui::TextUnformatted(iter->AgentName.c_str());
// 			ImGui::TableNextColumn();
// 			ImGui::TextUnformatted(iter->AccountName.c_str());
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%u", iter->Profession);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%x", iter->Elite);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%hhu", iter->Subgroup);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%llu", iter->MasterUniqueId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%llu", iter->UniqueId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%hu", iter->InstanceId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%s", iter->UniqueId == mySelfId ? "true" : "false");
//
// 			ImGui::TableNextColumn();
// 			if (ImGui::SmallButton("remove") == true)
// 			{
// 				iter = myAgents.erase(iter);
// 			}
// 			else
// 			{
// 				iter++;
// 			}
// 			i++;
//
// 			ImGui::PopID();
// 		}
//
// 		ImGui::EndTable();
// 	}
// 	ImGui::End();
// }

// void CombatMock::DisplayEvents()
// {
// 	ImGui::SetNextWindowSize(ImVec2(900, -1), ImGuiCond_FirstUseEver);
// 	if (ImGui::Begin("Events", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar) == true)
// 	{
// 		ImGui::BeginTable("##EventTable", 8, ImGuiTableFlags_SizingStretchProp);
//
// 		for (const auto& label : { "Type", "Time", "Source Agent", "Destination Agent", "Skill Id", "Value", "Is Buff" })
// 		{
// 			ImGui::TableNextColumn();
// 			ImGui::TextUnformatted(label);
// 		}
// 		ImGui::TableNextColumn(); // empty label in remove column
//
// 		int i = 0;
// 		auto iter = myEvents.begin();
// 		while (iter != myEvents.end())
// 		{
// 			ImGui::PushID(i); // Needed for button to work
//
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%s", CombatEventTypeString[static_cast<int>(iter->Type)]);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%llu", iter->Time);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%llu", iter->SourceAgentUniqueId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%llu", iter->DestinationAgentUniqueId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%u", iter->SkillId);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%i", iter->Value);
// 			ImGui::TableNextColumn();
// 			ImGui::Text("%s", iter->IsBuff == true ? "true" : "false");
//
// 			ImGui::TableNextColumn();
// 			if (ImGui::SmallButton("remove") == true)
// 			{
// 				iter = myEvents.erase(iter);
// 			}
// 			else
// 			{
// 				iter++;
// 			}
// 			i++;
//
// 			ImGui::PopID();
// 		}
//
// 		ImGui::EndTable();
// 	}
// 	ImGui::End();
// }

// void CombatMock::DisplayAddAgent()
// {
// 	ImGui::SetNextWindowSize(ImVec2(900, 70), ImGuiCond_Always);
// 	if (ImGui::Begin("Add agent", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar) == true)
// 	{
// 		ImGui::BeginTable("##AddAgent", 6, ImGuiTableFlags_SizingStretchSame, ImVec2(840, 70));
//
// 		for (const auto& label : { "Agent Name", "Account Name", "Profession", "Elite", "Subgroup", "Master Instance Id" })
// 		{
// 			ImGui::TableNextColumn();
// 			ImGui::TextUnformatted(label);
// 		}
//
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputText("##myInputAgentName", myInputAgentName, sizeof(myInputAgentName));
// 		ImGui::PopItemWidth();
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputText("##myInputAccountName", myInputAccountName, sizeof(myInputAccountName));
// 		ImGui::PopItemWidth();
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputInt("##myInputProfession", &myInputProfession, 0);
// 		ImGui::PopItemWidth();
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputInt("##myInputElite", &myInputElite, 0, 0, ImGuiInputTextFlags_CharsHexadecimal);
// 		ImGui::PopItemWidth();
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputInt("##myInputSubgroup", &myInputSubgroup, 0);
// 		ImGui::PopItemWidth();
// 		ImGui::TableNextColumn();
// 		ImGui::PushItemWidth(840 / 6);
// 		ImGui::InputScalar("##myInputMasterInstanceId", ImGuiDataType_U64, &myInputMasterUniqueId);
// 		ImGui::PopItemWidth();
//
// 		ImGui::EndTable();
//
// 		ImGui::SameLine();
// 		if (ImGui::Button("Add", ImVec2(-1, -1)) == true)
// 		{
// 			AddAgent(myInputAgentName, myInputAccountName, static_cast<Prof>(myInputProfession), static_cast<uint32_t>(myInputElite), static_cast<uint8_t>(myInputSubgroup), myInputMasterUniqueId);
// 		}
// 	}
// 	ImGui::End();
// }

// void CombatMock::DisplayAddEvent()
// {
// 	if (myInputTime == 0)
// 	{
// 		myInputTime = timeGetTime();
// 	}
// 	bool sourceAgentExists = (GetAgent(myInputSourceAgentUniqueId) != nullptr);
// 	bool destinationAgentExists = (GetAgent(myInputDestinationAgentUniqueId) != nullptr);
//
// 	ImGui::SetNextWindowSize(ImVec2(900, 70), ImGuiCond_Always);
// 	if (ImGui::Begin("Add event", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar) == true)
// 	{
// 		switch (static_cast<CombatEventType>(myInputCombatType))
// 		{
// 		case CombatEventType::Direct:
// 			ImGui::BeginTable("##AddEvent", 7, ImGuiTableFlags_SizingStretchSame, ImVec2(840, 70));
//
// 			for (const auto& label : { "Type", "Time", "Source Agent", "Destination Agent", "Skill Id", "Value", "Is Buff" })
// 			{
// 				ImGui::TableNextColumn();
// 				ImGui::TextUnformatted(label);
// 			}
//
// 			ImGui::TableNextColumn();
// 			ImGui::Combo("##myInputCombatType", &myInputCombatType, CombatEventTypeString, static_cast<int>(CombatEventType::Max));
//
// 			ImGui::TableNextColumn();
// 			ImGui::PushItemWidth(840 / 7);
// 			ImGui::InputScalar("##myInputTime", ImGuiDataType_U64, &myInputTime);
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
//
// 			ImGui::PushItemWidth(840 / 7);
// 			if (sourceAgentExists == false)
// 			{
// 				ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(191, 0, 0, 255));
// 			}
// 			ImGui::InputScalar("##myInputSourceAgentUniqueId", ImGuiDataType_U64, &myInputSourceAgentUniqueId);
// 			if (sourceAgentExists == false)
// 			{
// 				ImGui::PopStyleColor();
// 			}
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
//
// 			ImGui::PushItemWidth(840 / 7);
// 			if (destinationAgentExists == false)
// 			{
// 				ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(191, 0, 0, 255));
// 			}
// 			ImGui::InputScalar("##myInputDestinationAgentUniqueId", ImGuiDataType_U64, &myInputDestinationAgentUniqueId);
// 			if (destinationAgentExists == false)
// 			{
// 				ImGui::PopStyleColor();
// 			}
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
//
// 			ImGui::PushItemWidth(840 / 7);
// 			ImGui::InputScalar("##myInputSkillId", ImGuiDataType_U32, &myInputSkillId);
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
// 			ImGui::PushItemWidth(840 / 7);
// 			ImGui::InputScalar("##myInputValue", ImGuiDataType_S32, &myInputValue);
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
// 			ImGui::PushItemWidth(840 / 7);
// 			ImGui::Checkbox("##myInputIsBuff", &myInputIsBuff);
// 			ImGui::PopItemWidth();
//
// 			ImGui::EndTable();
//
// 			ImGui::SameLine();
// 			if (ImGui::Button("Add", ImVec2(-1, -1)) == true)
// 			{
// 				AddEvent(static_cast<CombatEventType>(myInputCombatType), myInputTime, myInputSourceAgentUniqueId, myInputDestinationAgentUniqueId, myInputSkillId, myInputValue, myInputIsBuff);
// 			}
// 			break;
// 		default:
// 			ImGui::BeginTable("##AddEvent", 3, ImGuiTableFlags_SizingStretchSame, ImVec2(360, 70));
//
// 			for (const auto& label : { "Type", "Time", "Agent"})
// 			{
// 				ImGui::TableNextColumn();
// 				ImGui::TextUnformatted(label);
// 			}
//
// 			ImGui::TableNextColumn();
// 			ImGui::PushItemWidth(360 / 3);
// 			ImGui::Combo("##myInputCombatType", &myInputCombatType, CombatEventTypeString, static_cast<int>(CombatEventType::Max));
// 			ImGui::PopItemWidth();
//
// 			ImGui::TableNextColumn();
// 			ImGui::PushItemWidth(360 / 3);
// 			ImGui::InputScalar("##myInputTime", ImGuiDataType_U64, &myInputTime);
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
//
// 			ImGui::PushItemWidth(360 / 3);
// 			if (sourceAgentExists == false)
// 			{
// 				ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(191, 0, 0, 255));
// 			}
// 			ImGui::InputScalar("##myInputSourceAgentUniqueId", ImGuiDataType_U64, &myInputSourceAgentUniqueId);
// 			if (sourceAgentExists == false)
// 			{
// 				ImGui::PopStyleColor();
// 			}
// 			ImGui::PopItemWidth();
// 			ImGui::TableNextColumn();
//
// 			ImGui::EndTable();
//
// 			ImGui::SameLine();
// 			if (ImGui::Button("Add", ImVec2(-1, -1)) == true)
// 			{
// 				AddEvent(static_cast<CombatEventType>(myInputCombatType), myInputTime, myInputSourceAgentUniqueId, 0, 0, 0, false);
// 			}
// 			break;
// 		}
// 	}
// 	ImGui::End();
// }

// void CombatMock::DisplayLog()
// {
// 	ImGui::Begin("Logs", &showLog, ImGuiWindowFlags_NoCollapse);
// 	if (ImGui::BeginPopupContextWindow() == true)
// 	{
// 		ImGui::InputInt("Lines to keep", &linesToKeep);
// 		ImGui::Checkbox("show filelog", &showFileLog);
// 		if (ImGui::Button("copy to clipboard") == true) {
// 			std::string text;
// 			if (showFileLog == true) {
// 				for (const auto& logLine : e3_log) {
// 					text.append(logLine);
// 					text.append("\n");
// 				}
// 			} else {
// 				for (const auto& logLine : e8_log) {
// 					text.append(logLine);
// 					text.append("\n");
// 				}
// 			}
// 			if (OpenClipboard(NULL)) {
// 				EmptyClipboard();
// 				HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, text.size() + 1);
// 				char* buffer = (char*)GlobalLock(clipbuffer);
// 				memset(buffer, 0, text.size() + 1);
// 				text.copy(buffer, text.size());
// 				GlobalUnlock(clipbuffer);
// 				SetClipboardData(CF_TEXT, clipbuffer);
// 				CloseClipboard();
// 			}
// 		}
//
// 		ImGui::EndPopup();
// 	}
//
// 	if (showFileLog == true)
// 	{
// 		for (const auto& logLine : e3_log)
// 		{
// 			ImGui::TextUnformatted(logLine.c_str());
// 		}
// 	}
// 	else
// 	{
// 		for (const auto& logLine : e8_log)
// 		{
// 			ImGui::TextUnformatted(logLine.c_str());
// 		}
// 	}
//
// 	ImGui::End();
// }

// void CombatMock::DisplayActions()
// {
// 	ImGui::SetNextWindowSize(ImVec2(360, 180), ImGuiCond_Always);
// 	if (ImGui::Begin("Actions", nullptr) == true)
// 	{
// 		bool selfAgentExists = (GetAgent(mySelfId) != nullptr);
// 		if (selfAgentExists == false)
// 		{
// 			ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(191, 0, 0, 255));
// 		}
// 		ImGui::InputScalar("Self Unique Id", ImGuiDataType_U64, &mySelfId);
// 		if (selfAgentExists == false)
// 		{
// 			ImGui::PopStyleColor();
// 		}
// 		ImGui::Separator();
//
// 		ImGui::InputText("File Path", myInputFilePath, sizeof(myInputFilePath));
// 		if (ImGui::Button("Save mock file") == true)
// 		{
// 			SaveToFile(myInputFilePath);
// 		}
// 		if (ImGui::Button("Load mock file") == true)
// 		{
// 			LoadFromFile(myInputFilePath);
// 		}
// 		if (ImGui::Button("Load and simulate xevtc") == true)
// 		{
// 			ExecuteFromXevtc(myInputFilePath, 0, 0);
// 		}
// 		ImGui::Separator();
//
// 		if (ImGui::Button("Start Simulation") == true)
// 		{
// 			Execute();
// 		}
// 	}
// 	ImGui::End();
// }

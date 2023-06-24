#define _CRT_SECURE_NO_WARNINGS

#include "CombatMock.h"

#include "Xevtc.h"

#include <nlohmann/json.hpp>

#include <atomic>
#include <cassert>
#include <Windows.h>
#include <fstream>
#include <iostream>

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

	std::ifstream inputStream(pFilePath, std::ios_base::in | std::ios_base::binary);

	return ExecuteFromXevtc(inputStream, pMaxParallelEventCount, pMaxFuzzWidth);
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

uint32_t
CombatMock::ExecuteFromXevtc(std::istream& pInputData, uint32_t pMaxParallelEventCount, uint32_t pMaxFuzzWidth) {
	// read throws an exception if it fails
	pInputData.exceptions(std::ios_base::failbit);

	// Header
	XevtcHeader header;
	pInputData.read(reinterpret_cast<char*>(&header), sizeof header);

	// Strings
	mXevtcStrings.clear();
	mXevtcStrings.reserve(header.StringCount);

	for (uint32_t i = 0; i < header.StringCount; i++)
	{
		uint16_t size;
		pInputData.read(reinterpret_cast<char*>(&size), sizeof(uint16_t));

		std::string& newString = mXevtcStrings.emplace_back();
		newString.resize(size);

		if (size != 0) // null strings are allowed in the xevtc
		{
			pInputData.read(newString.data(), size);
		}

		LOG("Parsed string %u %hu %s", i, size, newString.c_str());
	}

	std::vector<XevtcEvent> eventsVector(header.EventCount);
	pInputData.read(reinterpret_cast<char*>(eventsVector.data()), sizeof(XevtcEvent) * header.EventCount);

	// run events
	for (const auto& event : eventsVector) {
		ExecuteXevtcEvent(event, mXevtcStrings, *myCallbacks);
	}
	LOG("Simulated %u events", header.EventCount);
	return 0;
}

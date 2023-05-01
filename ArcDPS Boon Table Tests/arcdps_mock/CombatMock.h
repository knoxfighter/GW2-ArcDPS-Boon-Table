#pragma once

#include <ArcdpsExtension/arcdps_structs.h>

#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

class CombatMock
{
private:
	struct Agent
	{
		std::string AgentName;
		std::string AccountName;
		Prof Profession = PROF_UNKNOWN;
		uint32_t Elite = 0;
		uint8_t Subgroup = 0;
		uint64_t MasterUniqueId = 0;

		uint64_t UniqueId = 0;
		uint16_t InstanceId = 0;
	};

	enum class CombatEventType : int
	{
		Direct = 0,
		EnterCombat = 1,
		ExitCombat = 2,

		Max
	};

	static constexpr const char* const CombatEventTypeString[] = { "Direct", "EnterCombat", "ExitCombat" };

	struct CombatEvent
	{
		CombatEventType Type = CombatEventType::Direct;
		uint64_t Time = 0;
		uint64_t SourceAgentUniqueId = 0;
		uint64_t DestinationAgentUniqueId = 0;
		uint32_t SkillId = 0;
		int32_t Value = 0;
		bool IsBuff = false; // For now, this indicates whether the direct event came from condition/regeneration or from direct hit
	};

public:
	CombatMock(const arcdps_exports* pCallbacks);

	void AddAgent(const char* pAgentName, const char* pAccountName, Prof pProfession, uint32_t pElite, uint8_t pSubgroup, uint64_t pMasterUniqueId);
	void AddEvent(CombatEventType pType, uint64_t pTime, uint64_t pSourceAgentUniqueId, uint64_t pDestinationAgentUniqueId, uint32_t pSkillId, int32_t pValue, bool pIsBuff);

	uint32_t SaveToFile(const char* pFilePath);
	uint32_t LoadFromFile(const char* pFilePath);
	void Execute();

	// pMaxParallelEventCount determines how many events can be callbacked in parallel. 0 means synchronous callback.
	// pMaxFuzzWidth determines the maximum amount of indexes by which an events callback can be delayed (it is delayed
	//               by calling events ahead of it). 0 means no fuzzing.
	uint32_t ExecuteFromXevtc(const char* pFilePath, uint32_t pMaxParallelEventCount = 0, uint32_t pMaxFuzzWidth = 0);

	void DisplayWindow();

	// logging
	void e8LogLine(const std::string& line);
	void e3LogLine(const std::string& line);
	bool showLog = false;

private:
	void FillAgent(const Agent* pAgent, uint64_t pSelfId, ag& pResult);
	void FillAgentEvent(const Agent& pAgent, uint64_t pSelfId, ag& pSource, ag& pDestination);

	// Pointer is invalidated by the same things as a vector iterator
	const Agent* GetAgent(uint64_t pAgentUniqueId);

	void DisplayAgents();
	void DisplayEvents();
	void DisplayAddAgent();
	void DisplayAddEvent();
	void DisplayActions();
	void DisplayLog();

	const arcdps_exports* const myCallbacks;

	std::vector<Agent> myAgents;
	std::vector<CombatEvent> myEvents;
	uint64_t myNextUniqueId = 100000;
	uint16_t myNextInstanceId = 1;
	uint64_t mySelfId = 0;

	char myInputAgentName[256] = {};
	char myInputAccountName[256] = {};
	int myInputProfession = 0;
	int myInputElite = 0;
	int myInputSubgroup = 0;
	uint64_t myInputMasterUniqueId = 0;

	int myInputCombatType = 0;
	uint64_t myInputTime = 0;
	uint64_t myInputSourceAgentUniqueId = 0;
	uint64_t myInputDestinationAgentUniqueId = 0;
	uint32_t myInputSkillId = 0;
	int32_t myInputValue = 0;
	bool myInputIsBuff = false;

	char myInputFilePath[512] = {};

	std::map<uint32_t, std::string> mySkillNames;
	std::vector<std::string> mXevtcStrings;

	// logs window
	std::list<std::string> e3_log; //filelog
	std::list<std::string> e8_log; //ingamelog
	bool showFileLog = false;
	int linesToKeep = 50;
};
#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <chrono>

#include "TrackerHistory.h"
#include <ArcdpsExtension/arcdps_structs_slim.h>

class History {
private:
	using EntryType = std::deque<TrackerHistory>;
	// this holds the last X of fights as history
	EntryType entries;
	std::mutex entriesMutex;
	std::mutex historyMutex;

public:
	void LogStart(cbtevent* event);
	void LogEnd(cbtevent* event);
	void Event(ag* dst);
	void Reset(cbtevent* event);
	std::optional<size_t> GetTrackerIndexById(uint64_t trackerId);

	TrackerHistory& operator[](EntryType::size_type val) {
		std::lock_guard<std::mutex> guard(entriesMutex);
		return entries[val];
	}

	std::vector<std::reference_wrapper<TrackerHistory>> GetIteratorCopy();

private:
	enum class Status {
		Empty,
		WaitingForName,
		WaitingForReset,
		NameAcquired
	};
	uintptr_t currentID = 0;
	Status status = Status::Empty;
	std::string currentName;
	uint32_t currentBeginTimestamp = 0;
	std::chrono::hh_mm_ss<std::chrono::system_clock::duration> beginTimestamp;
	uint64_t trackerIdCounter = 0;
};

extern History history;

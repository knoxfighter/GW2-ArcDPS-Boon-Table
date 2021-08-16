#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <chrono>

#include "Lang.h"
#include "TrackerHistory.h"
#include "extension/arcdps_structs_slim.h"

class History {
private:
	using EntryType = std::deque<TrackerHistory>;
	// this holds the last X of fights as history
	EntryType entries;

public:
	void LogStart(cbtevent* event);
	void LogEnd(cbtevent* event);
	void Event(ag* dst);
	void Reset(cbtevent* event);

	[[nodiscard]] EntryType::const_iterator begin() const {
		return entries.begin();
	}
	[[nodiscard]] EntryType::const_iterator end() const {
		return entries.end();
	}
	EntryType::iterator begin() {
		return entries.begin();
	}
	EntryType::iterator end() {
		return entries.end();
	}
	TrackerHistory& operator[](EntryType::size_type val) {
		return entries[val];
	}

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
};

extern History history;

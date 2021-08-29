#include "History.h"

#include "Settings.h"
#include "Tracker.h"

History history;

void History::LogStart(cbtevent* event) {
	if (status != Status::Empty) {
		return;
	}

	if (isWvW && event->src_agent == 1) {
		status = Status::NameAcquired;
		currentName = lang.translate(LangKey::WvW);
	} else if (event->src_agent == 1) {
		// id for pre-events (e.g. Deimos)
		status = Status::WaitingForReset;
	} else {
		status = Status::WaitingForName;
		currentID = event->src_agent;
	}

	currentBeginTimestamp = event->buff_dmg;

	auto milliseconds = std::chrono::milliseconds(currentBeginTimestamp);
	auto sinceEpoch = std::chrono::duration_cast<std::chrono::system_clock::duration>(milliseconds);
	beginTimestamp = std::chrono::hh_mm_ss(sinceEpoch);
}

void History::LogEnd(cbtevent* event) {
	if (status == Status::NameAcquired) {
		// get fight duration
		uint32_t duration = event->buff_dmg - currentBeginTimestamp;

		// save stuff to historylist
		entries.emplace_front(liveTracker, duration, beginTimestamp, currentName);

		if (entries.size() > settings.getFightsToKeep()) {
			entries.pop_back();
		}
	}

	status = Status::Empty;
	currentID = 0;
	currentName = lang.translate(LangKey::Unknown);
	currentBeginTimestamp = 0;
}

void History::Event(ag* dst) {
	if (status != Status::WaitingForName) {
		return;
	}

	if (dst && dst->prof == currentID && dst->name) {
		currentName = dst->name;
		status = Status::NameAcquired;
	}
}

/// Called on `STATRESET`. Contains actual boss ID if not given in `LOGSTART`.
void History::Reset(cbtevent* event) {
	if (status != Status::WaitingForReset) {
		return;
	}

	status = Status::WaitingForName;
	currentID = event->src_agent;
}

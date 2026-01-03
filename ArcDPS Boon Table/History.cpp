#include "History.h"

#include "Lang.h"
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

	auto seconds = std::chrono::seconds(currentBeginTimestamp);
	auto timePoint = std::chrono::system_clock::time_point(seconds);
	auto timeT = std::chrono::system_clock::to_time_t(timePoint);
	tm tm;
	// convert to local time. 0 as return value means success.
	if (localtime_s(&tm, &timeT) == 0)
	{
		beginTimestamp = std::chrono::hh_mm_ss<std::chrono::system_clock::duration>(std::chrono::hours(tm.tm_hour) + std::chrono::minutes(tm.tm_min) + std::chrono::seconds(tm.tm_sec));
	}
	else
	{
		// fallback to UTC time in case of error 
		beginTimestamp = std::chrono::hh_mm_ss<std::chrono::system_clock::duration>(seconds);
	}
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
	std::lock_guard<std::mutex> guard(historyMutex);
	
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

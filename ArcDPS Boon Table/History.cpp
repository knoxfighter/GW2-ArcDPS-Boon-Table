#include "History.h"

#include "Lang.h"
#include "Settings.h"
#include "Tracker.h"

#include <ranges>

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

	std::chrono::seconds seconds(event->value);
	beginTimestamp = std::chrono::system_clock::time_point(seconds);
}

void History::LogEnd(cbtevent* event) {
	if (status == Status::NameAcquired) {
		std::lock_guard guard(entriesMutex);
		// get fight duration
		std::chrono::seconds seconds(event->value);
		std::chrono::system_clock::time_point endTimestamp(seconds);
		std::chrono::system_clock::duration duration = endTimestamp - beginTimestamp;

		// save stuff to historylist
		entries.emplace_front(liveTracker, duration, beginTimestamp, currentName, ++trackerIdCounter);

		if (entries.size() > settings.getFightsToKeep()) {
			entries.pop_back();
		}
	}

	status = Status::Empty;
	currentID = 0;
	currentName = lang.translate(LangKey::Unknown);
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

std::optional<size_t> History::GetTrackerIndexById(uint64_t trackerId)
{
	// Find the index of the entries object with Id set to trackerId
	for (const auto [i, entry] : entries | std::views::enumerate)
	{
		if (entry.getId() == trackerId)
		{
			return i;
		}
	}

	return std::nullopt;
}

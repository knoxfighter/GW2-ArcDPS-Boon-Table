#include "History.h"

#include "Lang.h"
#include "Settings.h"
#include "Tracker.h"
#include "Lang.h"

#include <ranges>

History history;

using ArcdpsExtension::Localization;

void History::LogStart(cbtevent* event) {
	if (status != Status::Empty) {
		return;
	}

	// 2 if map log, 3 if boss log
	if (event->dst_agent != 3) {
		return;
	}

	// if we are in wvw, we can skip everything and start wvw logging
	if (isWvW) {
		status = Status::NameAcquired;
		currentName = Localization::STranslate(BT_Wvw);
	}
	// if not, we have to wait for the LogNpcUpdate Event, which contains the actual id.
	// that event might be called immediately or delayed, depending on the fight.
	else {
		// id for pre-events (e.g. Deimos)
		status = Status::WaitingForReset;
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
	currentName = Localization::STranslate(ArcdpsExtension::ET_Unknown);
}

void History::Event(ag* dst) {
	std::lock_guard guard(historyMutex);
	
	if (status != Status::WaitingForName) {
		return;
	}

	if (dst && dst->prof == currentID && dst->name) {
		currentName = dst->name;
		status = Status::NameAcquired;
	}
}

/// Called on `CBTS_LOGNPCUPDATE`. Contains actual boss ID.
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

#pragma once

#include "HistoryTracker.h"

#include <ArcdpsExtension/SimpleRingBuffer.h>
#include <ArcdpsExtension/Singleton.h>

class Tracker;

class History final : public RingBuffer<HistoryTracker>, public Singleton<History> {
public:
	explicit History()
		: RingBuffer<HistoryTracker>(10) {}

	// override the ambiguous call `Clear`
	void Clear() {
		RingBuffer<HistoryTracker>::Clear();
	}

// public:
// 	void Add(const Tracker& pTracker);
// 	void ChangeMax(size_t pNewMax);
// 	void Clear();
//
// private:
// 	RingBuffer<HistoryTracker> mTrackers {10};
};

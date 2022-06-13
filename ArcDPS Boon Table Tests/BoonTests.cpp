#include <gtest/gtest.h>

#include "Tracker.h"

GTEST_TEST(BoonTests, MightIntensity2s2s) {
	Tracker& tracker = Tracker::instance();

	// Add player to tracking
	ag trackAddSrc {
		"User1",
		2000,
		PROF_ELE,
		0,
		0,
		0
	};
	ag trackAddDst {
		"User1",
		2000,
		PROF_ELE,
		1,
		1,
		0
	};
	tracker.Event(nullptr, &trackAddSrc, &trackAddDst, nullptr, 0);

	// Give player 5 might
	cbtevent mightAddEvent;
	memset(&mightAddEvent, 0, sizeof cbtevent);
	mightAddEvent.buff = 1;
	mightAddEvent.skillid = 740;
	for (int i = 0; i < 5; ++i) {
		tracker.Event(&mightAddEvent, nullptr, &trackAddDst, nullptr, 0);
	}

	// Put player into combat
	tracker.SetTime(1);
	cbtevent beginCombatEvent;
	memset(&beginCombatEvent, 0, sizeof cbtevent);
	beginCombatEvent.is_statechange = CBTS_ENTERCOMBAT;
	beginCombatEvent.dst_agent = 1;
	beginCombatEvent.time = 1000; // 1s
	tracker.Event(&beginCombatEvent, &trackAddDst, &trackAddDst, nullptr, 0);

	// wait 2 seconds
	tracker.SetTime(3);

	// Remove all might
	cbtevent mightRemoveEvent;
	memset(&mightRemoveEvent, 0, sizeof cbtevent);
	mightRemoveEvent.is_buffremove = CBTB_SINGLE;
	mightRemoveEvent.time = 3000; // 3s
	mightRemoveEvent.skillid = 740;
	mightRemoveEvent.buff = 1;
	for (int i = 0; i < 4; ++i) {
		tracker.Event(&mightRemoveEvent, &trackAddDst, nullptr, nullptr, 0);
	}
	mightRemoveEvent.is_buffremove = CBTB_MANUAL;
	tracker.Event(&mightRemoveEvent, &trackAddDst, nullptr, nullptr, 0);

	// wait 2 seconds
	tracker.SetTime(5);

	// read out Intensity
	Tracker::PlayerReadLock guard(tracker.PlayersMutex);
	const auto& players = tracker.GetAllPlayer();
	double intensity = players[0].GetIntensity(Boons::Might);

	// result should be 2.5
	EXPECT_DOUBLE_EQ(intensity, 2.5);
}

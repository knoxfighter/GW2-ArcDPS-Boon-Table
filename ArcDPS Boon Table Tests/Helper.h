#pragma once

#include "GlobalObjects.h"
#include "History.h"
#include "ITracker.h"
#include "Tracker.h"

#include "arcdps_mock/CombatMock.h"

#include <gtest/gtest.h>

struct TestBoon {
	Boons Boon;
	double Value;
	double AbsError;
};
struct TestPlayer {
	std::string AccountName;
	std::string CharacterName;
	std::vector<TestBoon> UptimeValues;
	std::vector<TestBoon> IntensityValues;

	TestPlayer(const std::string& pAccountName, const std::string& pCharacterName, const std::vector<TestBoon>& pUptimeValues,
		const std::vector<TestBoon>& pIntensityValues)
		: AccountName(pAccountName),
		  CharacterName(pCharacterName),
		  UptimeValues(pUptimeValues),
		  IntensityValues(pIntensityValues) {}

	TestPlayer(const std::string& pAccountName, const char8_t* pCharacterName, const std::vector<TestBoon>& pUptimeValues,
		const std::vector<TestBoon>& pIntensityValues)
		: AccountName(pAccountName),
		  CharacterName(reinterpret_cast<const char*>(pCharacterName)),
		  UptimeValues(pUptimeValues),
		  IntensityValues(pIntensityValues) {}
};
struct TestGroup {
	uint8_t Subgroup;
	std::vector<TestBoon> UptimeValues;
	std::vector<TestBoon> IntensityValues;
};
struct TestTotal {
	std::vector<TestBoon> UptimeValues;
	std::vector<TestBoon> IntensityValues;
};
struct TestLog {
	uintptr_t EncounterId;
	uint32_t DurationMS;
	std::vector<TestPlayer> Players;
	std::vector<TestGroup> Subgroups;
	TestTotal Total;
};

inline void TestTracker(const ITracker& pTracker, const TestLog& pLog, const uint32_t pDurationAbsError = 100) {
	ASSERT_EQ(pTracker.GetEncounterId(), pLog.EncounterId);
	ASSERT_NEAR((double)pTracker.GetDuration(), pLog.DurationMS, pDurationAbsError);

	for (const auto& testPlayer : pLog.Players) {
		const IPlayer* player = pTracker.GetPlayer(testPlayer.AccountName);
		ASSERT_NE(player, nullptr) << "Player: " << testPlayer.AccountName;
		ASSERT_EQ(player->GetCharacterName(), testPlayer.CharacterName);

		for (const auto& intensityValue : testPlayer.IntensityValues) {
			double intensity = player->GetIntensity(intensityValue.Boon);
			EXPECT_NEAR(intensity, intensityValue.Value, intensityValue.AbsError) << "Player: " << player->GetAccountName() << " - Boon: " << magic_enum::enum_name(intensityValue.Boon);
		}

		for (const auto& uptimeValue : testPlayer.UptimeValues) {
			double uptime = player->GetUptime(uptimeValue.Boon);
			EXPECT_NEAR(uptime, uptimeValue.Value, uptimeValue.AbsError) << "Player: " << player->GetAccountName() << " - Boon: " << magic_enum::enum_name(uptimeValue.Boon);
		}
	}

	for (const auto& subgroup : pLog.Subgroups) {
		for (const auto& intensityValue : subgroup.IntensityValues) {
			double subgroupIntensity = pTracker.GetSubgroupIntensity(subgroup.Subgroup, intensityValue.Boon);
			EXPECT_NEAR(subgroupIntensity, intensityValue.Value, intensityValue.AbsError) << "Subgroup: " << static_cast<uint32_t>(subgroup.Subgroup) << " - Boon: " << magic_enum::enum_name(intensityValue.Boon);
		}

		for (const auto& uptimeValue : subgroup.UptimeValues) {
			double subgroupUptime = pTracker.GetSubgroupUptime(subgroup.Subgroup, uptimeValue.Boon);
			EXPECT_NEAR(subgroupUptime, uptimeValue.Value, uptimeValue.AbsError) << "Subgroup: " << static_cast<uint32_t>(subgroup.Subgroup) << " - Boon: " << magic_enum::enum_name(uptimeValue.Boon);
		}
	}

	for (const auto& intensityValue : pLog.Total.IntensityValues) {
		double totalIntensity = pTracker.GetTotalIntensity(intensityValue.Boon);
		EXPECT_NEAR(totalIntensity, intensityValue.Value, intensityValue.AbsError) << "Boon: " << magic_enum::enum_name(intensityValue.Boon);
	}

	for (const auto& uptimeValue : pLog.Total.UptimeValues) {
		double totalUptime = pTracker.GetTotalUptime(uptimeValue.Boon);
		EXPECT_NEAR(totalUptime, uptimeValue.Value, uptimeValue.AbsError) << "Boon: " << magic_enum::enum_name(uptimeValue.Boon);
	}
}

using namespace std::chrono_literals;

#define HISTORY_TEST(pClassname, pXevtcfile) \
class pClassname : public testing::Test { \
protected: \
	static void SetUpTestSuite() { \
		Tracker& tracker = Tracker::instance(); \
		tracker.Reset(); \
		History& history = History::instance(); \
		history.Clear(); \
 \
		CombatMock mock(mod_init()); \
 \
		if (mock.ExecuteFromXevtc(pXevtcfile)) { \
			GTEST_FAIL(); \
		} \
		while(tracker.EventsPending()) { \
			std::this_thread::sleep_for(100ms); \
		} \
	} \
 \
	static void TearDownTestSuite() { \
		Tracker::instance().Reset(); \
		History::instance().Clear(); \
	} \
};

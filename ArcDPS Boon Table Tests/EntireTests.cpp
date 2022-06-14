#include "GlobalObjects.h"
#include "Tracker.h"

#include "arcdps_mock/arcdps_mock/CombatMock.h"

#include <gtest/gtest.h>

TEST(EntireTests, MightEfficiency) {
	CombatMock mock(&arc_exports);

	mock.ExecuteFromXevtc("D:\\ProgrammeSelbst\\C++\\boon-table\\ArcDPS Boon Table Tests\\logs\\generalTest.xevtc");

	const auto& players = Tracker::instance().GetAllPlayer();
	double mightIntensity = players[0].GetIntensity(Boons::Might);
	EXPECT_NEAR(mightIntensity, 7.952, 0.001);
}

#pragma once

#include "History.h"
#include "Logger.h"
#include "Tracker.h"

#include "arcdps_mock/CombatMock.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace HistoryTests {
	inline static std::map<std::string, std::filesystem::path> FILE_PATHS;
	inline static const char* CURRENT_SUITE_NAME;
}

class HistoryTestFixture : public testing::Test {
public:
	static void SetUpTestSuite() {
		using namespace std::chrono_literals;

		Tracker& tracker = Tracker::instance();
		tracker.Reset();
		History& history = History::instance();
		history.Clear();
		history.Resize(30);

		CombatMock mock(mod_init());

		const auto& xevtcPath = HistoryTests::FILE_PATHS.at(HistoryTests::CURRENT_SUITE_NAME);
		std::string xevtcPathStr = xevtcPath.string();

		if (mock.ExecuteFromXevtc(xevtcPathStr.c_str())) {
			GTEST_FAIL();
		}
		while(tracker.EventsPending()) {
			std::this_thread::sleep_for(100ms);
		}
	}

	static void TearDownTestSuite() {
		Tracker::instance().Reset();
		History::instance().Clear();
	}

};

class HistoryTest : public HistoryTestFixture {
public:
	explicit HistoryTest(const std::filesystem::path& pJsonPath, size_t pHistoryIndex)
		: mJsonPath(pJsonPath), mHistoryIndex(pHistoryIndex) {}

private:
	std::filesystem::path mJsonPath;
	size_t mHistoryIndex;

	void TestBody() override;
	// return: 1. uptime   2. intensity
	std::tuple<double, double> GetLogUptimes(const nlohmann::json& pJson, Boons pBoon, uintptr_t pEncounterId);
	static uint64_t GetFightDuration(const nlohmann::json& pJson, uintptr_t pEncounterId);
	static uint64_t GetPhaseNumber(uintptr_t pEncounterId);
};

class HistoryTestsEventListener : public ::testing::EmptyTestEventListener {
public:
	void OnTestSuiteStart(const testing::TestSuite& pTestSuite) override {
		HistoryTests::CURRENT_SUITE_NAME = pTestSuite.name();
	}
};

void RegisterHistoryTests();

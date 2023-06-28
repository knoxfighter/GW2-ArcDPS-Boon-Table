#pragma once

#include "History.h"
#include "Logger.h"
#include "Tracker.h"

#include "arcdps_mock/CombatMock.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <libzippp/libzippp.h>
#include <magic_enum.hpp>

#include <filesystem>
#include <spanstream>
#include <span>

namespace HistoryTests {
	inline static std::filesystem::path TEST_LOGS_ZIP(std::filesystem::absolute("../../ArcDPS Boon Table Tests/TestLogs/HistoryTests.zip"));
	inline static std::filesystem::path TEST_LOGS_FOLDER(std::filesystem::absolute("../../ArcDPS Boon Table Tests/TestLogs/HistoryTests"));

	enum class TestDataSource {
		ZIP,
		FILE,
	};

	struct TestData {
		TestDataSource source;
		std::filesystem::path filePath;
	};
	inline static std::map<std::string, TestData> STATIC_TEST_DATA;
	inline static const char* CURRENT_SUITE_NAME;
}

class HistoryTestFixture : public testing::Test {
public:
	static void SetUpTestSuite();
	static void TearDownTestSuite() {
		Tracker::instance().Reset();
		History::instance().Clear();
	}
};

class HistoryTest : public HistoryTestFixture {
public:
	explicit HistoryTest(const std::filesystem::path& pJsonPath, size_t pHistoryIndex, HistoryTests::TestDataSource pDataSource)
		: mJsonPath(pJsonPath), mHistoryIndex(pHistoryIndex), mDataSource(pDataSource) {}

private:
	std::filesystem::path mJsonPath;
	size_t mHistoryIndex;
	HistoryTests::TestDataSource mDataSource;

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

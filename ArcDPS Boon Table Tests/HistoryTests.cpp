#include "HistoryTests.h"

#include <ArcdpsExtension/MobIDs.h>

#include <nlohmann/json.hpp>
#include <libzippp/libzippp.h>

#include <filesystem>
#include <fstream>

void HistoryTest::TestBody() {
	// extract json from zip
	libzippp::ZipArchive zipFile(HistoryTests::LOG_FOLDER.string());
	ASSERT_TRUE(zipFile.open());

	auto jsonEntry = zipFile.getEntry(mJsonPath.generic_string());

	std::vector<char> buffer;
	buffer.resize(jsonEntry.getSize());
	std::span<char> bufferSpan(buffer);
	std::spanstream bufferStream(bufferSpan, std::ios_base::in | std::ios_base::out |std::ios_base::binary);
	auto res = jsonEntry.readContent(bufferStream);
	if (res != LIBZIPPP_OK) {
		GTEST_FAIL() << strerror(errno);
	}

	// parse json
	const auto& root = nlohmann::json::parse(bufferStream);

	const ITracker& tracker = History::instance()[mHistoryIndex];
	const auto logTriggerID = root.at("triggerID").get<uintptr_t>();

	// ignore River and Eyes logs
	if (
		tracker.GetEncounterId() == 19828 // skip river
		|| tracker.GetEncounterId() == 19651 // skip statues
		|| logTriggerID == static_cast<uintptr_t>(-1) // skip unknown
		|| logTriggerID == 16253 // skip escord
		|| logTriggerID == 16247 // skip twisted castle
	) GTEST_SKIP();

	// check encounter
	ASSERT_EQ(tracker.GetEncounterId(), root.at("triggerID").get<uintptr_t>());

	// check duration (get duration diff)
	uint64_t duration = tracker.GetDuration();
	uint64_t logDuration = GetFightDuration(root, tracker.GetEncounterId());
	double durationDiff = std::abs(((double)duration - (double)logDuration) / (double)duration) * 100;

	// add allowed diff (this is a hardcoded value to get around rounding and timing issues)
	// durationDiff += 0.05;
	durationDiff += 0.1;

	RecordProperty("DurationDiff", std::to_string(durationDiff));
	RecordProperty("EncounterId", tracker.GetEncounterId());
	std::cout << "DurationDiff: " << durationDiff << std::endl;

	// output duration diff to gtest
	// ASSERT_NEAR((double)duration, (double)logDuration, 100);

	// check boons on all players
	for (const auto& logPlayer : root.at("players")) {
		// ignore fake players (e.g. CA Swords)
		if (logPlayer.at("isFake").get<bool>()) continue;
		// ignore friendly NPCs (e.g. Saul on Deimos)
		if (logPlayer.at("friendlyNPC").get<bool>()) continue;

		std::string accountName = logPlayer.at("account").get<std::string>();
		std::string charName = logPlayer.at("name").get<std::string>();

		SCOPED_TRACE(accountName);

		const IPlayer* player = tracker.GetPlayer(accountName);

		ASSERT_TRUE(player);

		const auto& logBuffUptimes = logPlayer.at("buffUptimes");

		// iterate over all boons
		magic_enum::enum_for_each<Boons>([&](Boons boons) {
			SCOPED_TRACE(magic_enum::enum_name(boons));
			double uptime = player->GetUptime(boons);
			double intensity = player->GetIntensity(boons);
			auto [logUptime, logIntensity] = GetLogUptimes(logBuffUptimes, boons, tracker.GetEncounterId());

			EXPECT_NEAR(uptime, logUptime, durationDiff);
			EXPECT_NEAR(intensity, logIntensity, durationDiff);
		});
	}

	// TODO
	// check boons in all subgroups
	// check boons overall
}

std::tuple<double, double> HistoryTest::GetLogUptimes(const nlohmann::json& pJson, Boons pBoon, uintptr_t pEncounterId) {
	for (const auto& logBoon : pJson) {
		std::optional<Boons> boons = GetBoonsFromId(logBoon.at("id").get<uint32_t>());
		if (boons) {
			if (boons.value() == pBoon) {
				const auto& buffData = logBoon.at("buffData").at(GetPhaseNumber(pEncounterId));
				double uptime = buffData.at("uptime").get<double>();
				double presence = buffData.at("presence").get<double>();
				if (presence == 0) {
					return {uptime, 0};
				}
				return {presence, uptime};
			}
		}
	}
	return {0, 0};
}

uint64_t HistoryTest::GetFightDuration(const nlohmann::json& pJson, uintptr_t pEncounterId) {
	const auto& phases = pJson.at("phases").at(GetPhaseNumber(pEncounterId));
	auto start = phases.at("start").get<uint64_t>();
	auto end = phases.at("end").get<uint64_t>();
	return end - start;
}

uint64_t HistoryTest::GetPhaseNumber(uintptr_t pEncounterId) {
	switch (pEncounterId) {
	case std::to_underlying(TargetID::Dhuum):
	case std::to_underlying(TargetID::Xera):
	case std::to_underlying(TargetID::Deimos):
		return 2;
	default:
		return 0;
	}
}

void RegisterHistoryTests() {
//	std::filesystem::path path("../../ArcDPS Boon Table Tests/TestLogs/HistoryTests3.zip");
//	std::filesystem::path folderPath("../../ArcDPS Boon Table Tests/TestLogs");
//	libzippp::ZipArchive zipFile(path.string());
//	if (!zipFile.open(libzippp::ZipArchive::Write)) {
//		std::cout << "Failed to create zip file." << std::endl;
//		return;
//	}
//
//	for (const auto& entry : std::filesystem::recursive_directory_iterator(folderPath / "HistoryTests")) {
//		if (!entry.is_directory()) {
//			const std::filesystem::path& filePath = entry.path();
//			std::string relativePath = std::filesystem::relative(filePath, folderPath).generic_string();
//			assert(zipFile.addFile(relativePath, filePath.generic_string()));
//			std::cout << "Added " << relativePath << " to the zip-file." << std::endl;
//		}
//	}
//
//	zipFile.close();
//	std::cout << "Folder compressed successfully." << std::endl;

	::testing::UnitTest::GetInstance()->listeners().Append(new HistoryTestsEventListener);

	std::filesystem::path logFolder("../../ArcDPS Boon Table Tests/TestLogs/HistoryTests");

	libzippp::ZipArchive zipFile(HistoryTests::LOG_FOLDER.string());
	if (!zipFile.open()) {
		std::cout << "Failed to open zip archive." << std::endl;
		assert(false);
	}

	std::unordered_map<std::string, std::vector<std::filesystem::path>> tests;
	std::vector<libzippp::ZipEntry> entries = zipFile.getEntries();

	for (const auto& entry : entries) {
		std::filesystem::path filePath = entry.getName();
		// check if file is a xevtc file
		if (!entry.isDirectory() && filePath.extension() == ".json") {
			std::string testName = filePath.parent_path().filename().string();

			tests[testName].emplace_back(filePath);
		} else if (!entry.isDirectory() && filePath.extension() == ".xevtc") {
			std::string testName = filePath.parent_path().filename().string();

			// add xevtc file to static context
			HistoryTests::FILE_PATHS[testName] = filePath;
		}
	}
	zipFile.close();

	for (const auto& [testName, jsonPaths] : tests) {
		for (const auto& jsonPath : jsonPaths) {
			// get id and name to check (separated by `-`)
			std::string jsonFileName = jsonPath.stem().string();
			size_t first = jsonFileName.find_first_of('-');
			std::string name = jsonFileName.substr(first+1);

			std::string id = jsonFileName.substr(0, first);

			// register new test
			::testing::RegisterTest(
					testName.c_str(),
					name.c_str(), nullptr,
					nullptr,
					__FILE__,
					__LINE__,
					[=]() -> HistoryTestFixture* {
						return new HistoryTest(jsonPath, std::stoll(id));
					}
			);
		}
	}
}

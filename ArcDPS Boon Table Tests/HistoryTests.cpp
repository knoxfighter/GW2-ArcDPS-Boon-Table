#include "HistoryTests.h"

#include <ArcdpsExtension/MobIDs.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

void HistoryTest::TestBody() {
	// parse json
	std::ifstream jsonFile(mJsonPath);
	const auto& root = nlohmann::json::parse(jsonFile);

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
	::testing::UnitTest::GetInstance()->listeners().Append(new HistoryTestsEventListener);

//	auto p = std::filesystem::current_path();
//	std::cout << "The current path " << p << " decomposes into:\n"
//	          << "root-path " << p.root_path() << '\n'
//	          << "relative path " << p.relative_path() << '\n';
//	p = p.parent_path().parent_path();
//	p /= "ArcDPS Boon Table Tests/logs/HistoryTests";
	std::filesystem::path logFolder("../../ArcDPS Boon Table Tests/TestLogs/HistoryTests");

	for (const auto& subFolder : std::filesystem::directory_iterator(logFolder)) {
		if (subFolder.is_directory()) {
			// find xevtc file in subFolder (has to have the same name as the folder)
			// aka fixture name
			std::string folderName = subFolder.path().filename().string();

			auto xevtcFile = subFolder.path() / folderName;
			xevtcFile.replace_extension("xevtc");

			// add xevtc file to static context
			HistoryTests::FILE_PATHS[folderName] = xevtcFile;

			// find all json in the folder
			for (const auto& subFile : std::filesystem::directory_iterator(subFolder)) {
				// only use json files
				const auto& subFilePath = subFile.path();
				if (subFilePath.extension() == ".json") {
					// get id and name to check (separated by `-`)
					std::string jsonFileName = subFilePath.stem().string();
					size_t first = jsonFileName.find_first_of('-');
					std::string name = jsonFileName.substr(first+1);

					std::string id = jsonFileName.substr(0, first);

					// register new test
					::testing::RegisterTest(
						folderName.c_str(),
						name.c_str(), nullptr,
						nullptr,
						__FILE__,
						__LINE__,
						[=]() -> HistoryTestFixture* {
							return new HistoryTest(subFilePath, std::stoll(id));
						});
				}
			}
		}
	}
}

#include "HistoryTests.h"

#include <gtest/gtest.h>

#include <libzippp.h>

#include <span>
#include <spanstream>

TEST(GeneralTests, ShutdownTest) {
	CombatMock mock(mod_init());

	// open zip
	libzippp::ZipArchive zipFile(HistoryTests::TEST_LOGS_ZIP.string());
	ASSERT_TRUE(zipFile.open());

	const auto& xevtcPath = "HistoryTests/W1_1/W1_1.xevtc";
	auto xevtcEntry = zipFile.getEntry(xevtcPath);

	// create buffer, then a spanstream, so we have an in RAM iostream
	std::vector<char> buffer;
	buffer.resize(xevtcEntry.getSize());
	std::span<char> bufferSpan(buffer);
	std::spanstream bufferStream(bufferSpan, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	auto res = xevtcEntry.readContent(bufferStream);
	if (res != LIBZIPPP_OK) {
		GTEST_FAIL() << strerror(errno);
	}

	if (mock.ExecuteFromXevtc(bufferStream)) {
		GTEST_FAIL();
	}

	zipFile.close();

	mod_release();
	ASSERT_TRUE(g_singletonManagerInstance.Empty());
}

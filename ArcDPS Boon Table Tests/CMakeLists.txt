find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(ArcDPSBoonTableTests)

# Use -MT / -MTd runtime library
set_property(TARGET ArcDPSBoonTableTests PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_sources(ArcDPSBoonTableTests INTERFACE
    FILE_SET HEADERS
    FILES
        Helper.h
        HistoryTests.h
        arcdps_mock/CombatMock.h
        arcdps_mock/Xevtc.h
)

target_sources(ArcDPSBoonTableTests
    PRIVATE
        HistoryTests.cpp
        TestMain.cpp
        arcdps_mock/CombatMock.cpp
)

find_package(libzippp CONFIG REQUIRED)

boontable_link_libraries(ArcDPSBoonTableTests)
target_link_libraries(ArcDPSBoonTableTests PRIVATE GTest::gtest)
target_link_libraries(ArcDPSBoonTableTests PRIVATE ArcDPSBoonTableLib)
target_link_libraries(ArcDPSBoonTableTests PRIVATE libzip::zip libzippp::libzippp)

# Windows Libs
target_link_libraries(ArcDPSBoonTableTests PRIVATE Version.lib)
target_link_libraries(ArcDPSBoonTableTests PRIVATE Winmm.lib)

# download hash
file(DOWNLOAD "https://static.knox.moe/files/HistoryTests.zip.sha256" "${CMAKE_BINARY_DIR}/HistoryTests.zip.sha256")
file(STRINGS "${CMAKE_BINARY_DIR}/HistoryTests.zip.sha256" remote_test_hash)
file(REMOVE "${CMAKE_BINARY_DIR}/HistoryTests.zip.sha256")

# download zip file
file(DOWNLOAD "https://static.knox.moe/files/HistoryTests.zip" "${CMAKE_CURRENT_SOURCE_DIR}/TestLogs/HistoryTests.zip"
        EXPECTED_HASH SHA256=${remote_test_hash}
        SHOW_PROGRESS
        STATUS download_status)
message("${download_status}")

gtest_discover_tests(ArcDPSBoonTableTests)

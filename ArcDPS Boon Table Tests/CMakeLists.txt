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

target_link_libraries(ArcDPSBoonTableTests PRIVATE GTest::gtest)
target_link_libraries(ArcDPSBoonTableTests PRIVATE imgui::imgui)
target_link_libraries(ArcDPSBoonTableTests PRIVATE cpr::cpr)
target_link_libraries(ArcDPSBoonTableTests PRIVATE magic_enum::magic_enum)
target_link_libraries(ArcDPSBoonTableTests PRIVATE ArcdpsUnofficialExtras)
target_link_libraries(ArcDPSBoonTableTests PRIVATE ArcdpsExtension)
target_link_libraries(ArcDPSBoonTableTests PRIVATE ArcDPSBoonTableLib)

# Windows Libs
target_link_libraries(ArcDPSBoonTableTests PRIVATE Version.lib)
target_link_libraries(ArcDPSBoonTableTests PRIVATE Winmm.lib)

gtest_discover_tests(ArcDPSBoonTableTests)

cmake_minimum_required(VERSION 3.25)
project(d3d9_arcdps_table)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

find_package(magic_enum CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(ArcdpsUnofficialExtras CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ArcdpsExtension CONFIG REQUIRED)

set(SOURCES
    BigTable/BigTable.cpp
    modernIni/modernIni/modernIni.cpp
    AppChart.cpp
    "ArcDPS Boon Table.rc"
    Boon.cpp
    BuffIds.cpp
    Entity.cpp
    EntityHistory.cpp
    Helpers.cpp
    History.cpp
    Lang.cpp
    main.cpp
    NPC.cpp
    Player.cpp
    PlayerHistory.cpp
    Settings.cpp
    SettingsUI.cpp
    SettingsUIGlobal.cpp
    Tracker.cpp
    TrackerHistory.cpp
)

set(HEADERS
    modernIni/modernIni/map.h
    modernIni/modernIni/modernIni.h
    modernIni/modernIni/modernIniMacros.h
    AppChart.h
    Boon.h
    BuffIds.h
    Entity.h
    EntityHistory.h
    Helpers.h
    History.h
    IEntity.h
    IPlayer.h
    ITracker.h
    Lang.h
    NPC.h
    Player.h
    PlayerHistory.h
    resource.h
    Settings.h
    SettingsUI.h
    SettingsUIGlobal.h
    Tracker.h
    TrackerHistory.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Use -MT / -MTd runtime library
set_property(TARGET ${PROJECT_NAME} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/modernIni"
    "${CMAKE_CURRENT_SOURCE_DIR}/BigTable"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ArcdpsExtension::ArcdpsExtension
)

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)

if(MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
        DESTINATION .
        OPTIONAL
    )
endif()

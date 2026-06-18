cmake_minimum_required(VERSION 4.2)

include(FetchContent)
FetchContent_Declare(my-vcpkg-triplets
		GIT_REPOSITORY "https://github.com/Zinn-o-Matics/my-vcpkg-triplets.git"
		GIT_TAG "master"
)
FetchContent_MakeAvailable(my-vcpkg-triplets)

list(APPEND VCPKG_OVERLAY_TRIPLETS "${my-vcpkg-triplets_SOURCE_DIR}")

project(boon_table CXX)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

find_package(magic_enum CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(modernIni CONFIG REQUIRED)
find_package(ArcdpsExtension CONFIG REQUIRED)
find_package(ArcdpsUnofficialExtras CONFIG REQUIRED)

set(VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/VersionInfo.rc")
# add custom target to regenerate version info on every build
add_custom_target(VersionInfo ALL
		COMMAND ${CMAKE_COMMAND}
		-DVERSION_MAJOR=${VERSION_MAJOR}
		-DVERSION_MINOR=${VERSION_MINOR}
		-DVERSION_PATCH=${VERSION_PATCH}
		-DVERSION_REF=${VERSION_REF}
		-DVERSION_REF_STR=${VERSION_REF_STR}
		-DINTERNAL_NAME=d3d9_arcdps_table.dll
		-DORIGINAL_FILENAME=d3d9_arcdps_table.dll
		-DPRODUCT_NAME=Arcdps Boon table plugin
		-DDESCRIPTION=Arcdps Boon table plugin
		-P ${ArcdpsExtension_DIR}/generate-version.cmake
		BYPRODUCTS ${VERSION_FILE}
		COMMENT "Generating VersionInfo.rc"
		VERBATIM
)

add_library(${PROJECT_NAME} SHARED)

add_dependencies(${PROJECT_NAME} VersionInfo)

# Use -MT / -MTd runtime library
set_property(TARGET ${PROJECT_NAME} PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# rename output to legacy name
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "d3d9_arcdps_table")

# add general sources
target_sources(${PROJECT_NAME}
		PUBLIC
		FILE_SET HEADERS
		FILES
		"ArcDPS Boon Table/AppChart.h"
		"ArcDPS Boon Table/Boon.h"
		"ArcDPS Boon Table/BuffIds.h"
		"ArcDPS Boon Table/Entity.h"
		"ArcDPS Boon Table/EntityHistory.h"
		"ArcDPS Boon Table/Helpers.h"
		"ArcDPS Boon Table/History.h"
		"ArcDPS Boon Table/IEntity.h"
		"ArcDPS Boon Table/IPlayer.h"
		"ArcDPS Boon Table/ITracker.h"
		"ArcDPS Boon Table/KeysDown.h"
		"ArcDPS Boon Table/Lang.h"
		"ArcDPS Boon Table/Player.h"
		"ArcDPS Boon Table/PlayerHistory.h"
		"ArcDPS Boon Table/Settings.h"
		"ArcDPS Boon Table/SettingsUI.h"
		"ArcDPS Boon Table/SettingsUIGlobal.h"
		"ArcDPS Boon Table/Tracker.h"
		"ArcDPS Boon Table/TrackerHistory.h"

		"ArcDPS Boon Table/BigTable/BigTable.h"
)

target_sources(${PROJECT_NAME}
		PRIVATE
		"${VERSION_FILE}"
		"ArcDPS Boon Table/resource.h"
		"ArcDPS Boon Table/icons.rc"
		"ArcDPS Boon Table/AppChart.cpp"
		"ArcDPS Boon Table/Boon.cpp"
		"ArcDPS Boon Table/BuffIds.cpp"
		"ArcDPS Boon Table/Entity.cpp"
		"ArcDPS Boon Table/EntityHistory.cpp"
		"ArcDPS Boon Table/Helpers.cpp"
		"ArcDPS Boon Table/History.cpp"
		"ArcDPS Boon Table/Lang.cpp"
		"ArcDPS Boon Table/main.cpp"
		"ArcDPS Boon Table/Player.cpp"
		"ArcDPS Boon Table/PlayerHistory.cpp"
		"ArcDPS Boon Table/Settings.cpp"
		"ArcDPS Boon Table/SettingsUI.cpp"
		"ArcDPS Boon Table/SettingsUIGlobal.cpp"
		"ArcDPS Boon Table/Tracker.cpp"
		"ArcDPS Boon Table/TrackerHistory.cpp"

		"ArcDPS Boon Table/BigTable/BigTable.cpp"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PUBLIC magic_enum::magic_enum)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC modernIni::modernIni)
target_link_libraries(${PROJECT_NAME} PUBLIC ArcdpsExtension::ArcdpsExtension)
target_link_libraries(${PROJECT_NAME} PUBLIC ArcdpsUnofficialExtras)

if (DEFINED GAME_DIR AND NOT GAME_DIR STREQUAL "")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "${GAME_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug,RelWithDebInfo>,copy_if_different,true> $<TARGET_PDB_FILE:${PROJECT_NAME}> "${GAME_DIR}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
			COMMENT "Copying plugin to ${GAME_DIR}..."
	)
endif ()

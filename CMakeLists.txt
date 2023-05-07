cmake_minimum_required(VERSION 3.23)

project(ArcDPSBoonTable CXX)
set(CMAKE_CXX_STANDARD 23)

option(BUILD_TESTS "Build the GTest executable" OFF)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

add_compile_definitions(
    NOMINMAX
    MAGIC_ENUM_RANGE_MAX=256
    IMGUI_DEFINE_MATH_OPERATORS
)

find_package(ArcdpsUnofficialExtras CONFIG REQUIRED)
find_package(ArcdpsExtension CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_subdirectory("ArcDPS Boon Table Lib")
add_subdirectory("ArcDPS Boon Table")
add_subdirectory("ArcDPS Boon Table Tests")

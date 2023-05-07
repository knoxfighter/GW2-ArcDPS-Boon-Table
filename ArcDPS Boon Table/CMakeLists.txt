add_library(ArcDPSBoonTable SHARED)

# Use -MT / -MTd runtime library
set_property(TARGET ArcDPSBoonTable PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_sources(ArcDPSBoonTable
    PRIVATE
        dllmain.cpp
)

target_link_libraries(ArcDPSBoonTable PRIVATE imgui::imgui)
target_link_libraries(ArcDPSBoonTable PRIVATE cpr::cpr)
target_link_libraries(ArcDPSBoonTable PRIVATE magic_enum::magic_enum)
target_link_libraries(ArcDPSBoonTable PRIVATE ArcdpsUnofficialExtras)
target_link_libraries(ArcDPSBoonTable PRIVATE ArcdpsExtension)
target_link_libraries(ArcDPSBoonTable PRIVATE ArcDPSBoonTableLib)

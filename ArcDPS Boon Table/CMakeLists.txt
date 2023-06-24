add_library(ArcDPSBoonTable SHARED)

# Use -MT / -MTd runtime library
set_property(TARGET ArcDPSBoonTable PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_sources(ArcDPSBoonTable
    PRIVATE
        dllmain.cpp
)

boontable_link_libraries(ArcDPSBoonTable)
target_link_libraries(ArcDPSBoonTable PRIVATE ArcDPSBoonTableLib)

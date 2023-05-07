add_library(ArcDPSBoonTableLib STATIC)

# Use -MT / -MTd runtime library
set_property(TARGET ArcDPSBoonTableLib PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_sources(ArcDPSBoonTableLib INTERFACE
    FILE_SET HEADERS
    FILES
        Boon.h
        BoonDef.h
        BoonWindow.h
        BoonWindowHandler.h
        BoonWindowTable.h
        GlobalObjects.h
        History.h
        HistoryPlayer.h
        HistoryTracker.h
        IPlayer.h
        ITracker.h
        Logger.h
        Player.h
        Settings.h
        Tracker.h

        Activity/Activities.h
        Activity/IActivityTracker.h
        Activity/IffActivity.h
        Activity/IffNpcTracker.h
        Activity/NpcUpdateTracker.h
        Activity/RaidActivity.h
        Activity/RaidNpcTracker.h
        Activity/TestActivity.h
)

target_sources(ArcDPSBoonTableLib
    PRIVATE
        Boon.cpp
        BoonDef.cpp
        BoonWindow.cpp
        BoonWindowHandler.cpp
        BoonWindowTable.cpp
        GlobalObjects.cpp
        History.cpp
        HistoryPlayer.cpp
        HistoryTracker.cpp
        libmain.cpp
        Logger.cpp
        Player.cpp
        Settings.cpp
        Tracker.cpp

        Activity/IActivityTracker.cpp
        Activity/IffActivity.cpp
        Activity/NpcUpdateTracker.cpp
        Activity/RaidActivity.cpp
        Activity/RaidNpcTracker.cpp
)

target_link_libraries(ArcDPSBoonTableLib PRIVATE imgui::imgui)
target_link_libraries(ArcDPSBoonTableLib PRIVATE cpr::cpr)
target_link_libraries(ArcDPSBoonTableLib PRIVATE magic_enum::magic_enum)
target_link_libraries(ArcDPSBoonTableLib PRIVATE ArcdpsUnofficialExtras)
target_link_libraries(ArcDPSBoonTableLib PRIVATE ArcdpsExtension)

# Windows Libs
target_link_libraries(ArcDPSBoonTableLib PRIVATE Version.lib)

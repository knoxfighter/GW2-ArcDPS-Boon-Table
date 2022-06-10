#pragma once

#include "extension/arcdps_structs.h"
#include "extension/MumbleLink.h"
#include "extension/UpdateCheckerBase.h"

#include "unofficial_extras/Definitions.h"

#include <Windows.h>

class GlobalObjects {
public:
	// Updating myself stuff
	static inline std::unique_ptr<UpdateCheckerBase::UpdateState> UPDATE_STATE = nullptr;

	// arc keyboard modifier
	static inline DWORD ARC_GLOBAL_MOD1 = 0;
	static inline DWORD ARC_GLOBAL_MOD2 = 0;
	static inline DWORD ARC_GLOBAL_MOD_MULTI = 0;

	// Arc export Cache
	static inline bool ARC_HIDE_ALL = false;
	static inline bool ARC_PANEL_ALWAYS_DRAW = false;
	static inline bool ARC_MOVELOCK_ALTUI = false;
	static inline bool ARC_CLICKLOCK_ALTUI = false;
	static inline bool ARC_WINDOW_FASTCLOSE = false;

	// Arc helper functions
	static void UpdateArcExports();
	static bool ModsPressed();
	static bool CanMoveWindows();

	static inline LinkedMem* MUMBLE_MEM = nullptr;
	static inline Language CURRENT_LANGUAGE = Language::English;
	static inline HKL CURRENT_HKL;
	static inline HMODULE SELF_DLL;
};

// Exports
extern "C" __declspec(dllexport) ModInitSignature get_init_addr(const char* arcversion, ImGuiContext* imguictx, void* dxptr, HMODULE arcdll, MallocSignature mallocfn, FreeSignature freefn, UINT dxver);
extern "C" __declspec(dllexport) ModReleaseSignature get_release_addr();
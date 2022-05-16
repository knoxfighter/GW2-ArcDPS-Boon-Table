#pragma once

#include "unofficial_extras/Definitions.h"

#include <memory>
#include <Windows.h>

class GlobalObjects {
public:
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

	// other
	// TODO: fill with actual values and keep them up to date
	static inline Language CURRENT_LANGUAGE = Language::English;
	static inline HKL CURRENT_HKL;
};

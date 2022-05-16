#include "global.h"

#include "extension/arcdps_structs.h"

#include "imgui/imgui.h"

void GlobalObjects::UpdateArcExports() {
	uint64_t e6_result = ARC_EXPORT_E6();
	uint64_t e7_result = ARC_EXPORT_E7();

	ARC_HIDE_ALL = (e6_result & 0x01);
	ARC_PANEL_ALWAYS_DRAW = (e6_result & 0x02);
	ARC_MOVELOCK_ALTUI = (e6_result & 0x04);
	ARC_CLICKLOCK_ALTUI = (e6_result & 0x08);
	ARC_WINDOW_FASTCLOSE = (e6_result & 0x10);


	uint16_t* ra = (uint16_t*)&e7_result;
	if (ra) {
		ARC_GLOBAL_MOD1 = ra[0];
		ARC_GLOBAL_MOD2 = ra[1];
		ARC_GLOBAL_MOD_MULTI = ra[2];
	}
}

bool GlobalObjects::ModsPressed() {
	auto io = &ImGui::GetIO();

	return io->KeysDown[ARC_GLOBAL_MOD1] && io->KeysDown[ARC_GLOBAL_MOD2];
}

bool GlobalObjects::CanMoveWindows() {
	if (!ARC_MOVELOCK_ALTUI) {
		return true;
	}
	return ModsPressed();
}

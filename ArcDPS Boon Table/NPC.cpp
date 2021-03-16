#include "NPC.h"

NPC::NPC(uintptr_t new_id = 0,
	const std::string& new_name = "") {
	id = new_id;
	name = new_name;
}

bool NPC::operator==(std::string other_name) const {
    return name == other_name;
}

bool NPC::operator==(uintptr_t other_id) const {
	return id == other_id;
}

bool NPC::operator==(const Entity& other) const {
	return id == other.id && name == other.name;
}

ImVec4 NPC::getColor() const {
    /* e5 writes out colour array ptrs, sizeof(out) == sizeof(ImVec4*) * 5.  [ void e5(ImVec4** out) ]
       out[0] = core cols
                   enum n_colours_core {
                     CCOL_TRANSPARENT,
                     CCOL_WHITE,
                     CCOL_LWHITE,
                     CCOL_LGREY,
                     CCOL_LYELLOW,
                     CCOL_LGREEN,
                     CCOL_LRED,
                     CCOL_LTEAL,
                     CCOL_MGREY,
                     CCOL_DGREY,
                     CCOL_NUM
                   };
       out[1] = prof colours base
       out[2] = prof colours highlight
                   prof colours match prof enum
       out[3] = subgroup colours base
       out[4] = subgroup colours highlight
                   subgroup colours match subgroup, up to game max, out[3][15]
     */
    ImVec4* arc_colors[5];
    arc_export_e5(arc_colors);

    //all npcs are mesmers now!
    return arc_colors[2][PROF_MESMER];
}

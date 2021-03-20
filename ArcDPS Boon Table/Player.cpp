#include "Player.h"

Player::Player(uintptr_t new_id = 0,
	const std::string& new_name = "",
	const std::string& new_account_name = "",
	uint8_t new_subgroup = 1,
	Prof new_profession = PROF_UNKNOWN)
{
	id = new_id;
	name = new_name;
	account_name = new_account_name;
	enter_combat_time = getCurrentTime();
	in_combat = false;
	subgroup = new_subgroup;
	profession = new_profession;
}

bool Player::operator==(std::string other_name) const {
	return name == other_name
		|| account_name == other_name;
}

bool Player::operator==(uintptr_t other_id) const {
    return id == other_id;
}

bool Player::operator==(const Entity& other) const {
    return id == other.id && name == other.name;
}

void Player::combatEnter(cbtevent* ev) {
    subgroup = ev->dst_agent;
    Entity::combatEnter(ev);
}

ImVec4 Player::getColor() const {
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

    return arc_colors[1][profession];
}

#include "PlayerHistory.h"

uint8_t PlayerHistory::getSubgroup() const {
	return subgroup;
}

bool PlayerHistory::isSelf() const {
	return self;
}

Prof PlayerHistory::getProfession() const {
	return prof;
}

ImVec4 PlayerHistory::getColor() const {
	ImVec4* arc_colors[5];
    arc_export_e5(arc_colors);

    return arc_colors[1][prof];
}

#include "ArcdpsDataStructures.h"

bool is_player(ag* new_player)
{
	return new_player
		&& new_player->elite != 0xffffffff
		&& new_player->name
		&& std::string(new_player->name).length() > 1
		&& new_player->id;
}

std::string to_string(Alignment alignment) {
	switch (alignment) {
	case Alignment::Left: 
		return "Left";
	case Alignment::Center: 
		return "Center";
	case Alignment::Right: 
		return "Right";
	default: 
		return "Unaligned";
	}
}

std::string to_string(ProgressBarColoringMode coloringMode) {
	switch (coloringMode) {
	case ProgressBarColoringMode::Uncolored: 
		return "Default";
	case ProgressBarColoringMode::ByProfession:
		return "By Profession";
	case ProgressBarColoringMode::ByPercentage:
		return "By Percentage";
	}
}

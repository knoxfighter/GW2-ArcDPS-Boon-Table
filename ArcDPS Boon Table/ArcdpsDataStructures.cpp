#include "ArcdpsDataStructures.h"

bool is_player(ag* new_player)
{
	return new_player
		&& new_player->elite != 0xffffffff
		&& new_player->name
		&& new_player->id;
}
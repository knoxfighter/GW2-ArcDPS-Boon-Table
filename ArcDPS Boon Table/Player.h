#pragma once

#include <string>
#include <list>
#include <mutex>
#include "Boon.h"

class Player
{
public:
	uintptr_t id;
	std::string name;
	std::list<Boon> boons;

	bool operator==(uintptr_t other_id);

	Player();
	Player(uintptr_t new_id, std::string new_name);
	~Player();

	void applyBoon(uint16_t new_id,int32_t new_duration);
	void removeBoon(uint16_t new_id, int32_t new_duration);
};

extern std::mutex boons_mtx;
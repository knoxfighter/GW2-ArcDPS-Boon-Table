#include "extension/arcdps_structs_slim.h"

#include <gtest/gtest.h>

#include <Windows.h>

#pragma pack(push, 1)
namespace
{
struct ArcModifiers
{
	uint16_t _1 = VK_SHIFT;
	uint16_t _2 = VK_MENU;
	uint16_t Multi = 0;
	uint16_t Fill = 0;
};
} // anonymous namespace
#pragma pack(pop)

void e3(const char* /*pString*/)
{
	return; // Logging, ignored
}

uint64_t e6()
{
	return 0; // everything set to false
}

uint64_t e7()
{
	ArcModifiers mods;
	return *reinterpret_cast<uint64_t*>(&mods);
}

void e9(cbtevent*, uint32_t)
{
	return; // Ignore, can be overridden by specific test if need be
}

int main(int pArgumentCount, char** pArgumentVector)
{
	::testing::InitGoogleTest(&pArgumentCount, pArgumentVector); 

	int result = RUN_ALL_TESTS();

	return result;
}

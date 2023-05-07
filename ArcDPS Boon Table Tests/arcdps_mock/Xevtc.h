#pragma once
#include <ArcdpsExtension/arcdps_structs.h>

#include <cstdint>

#pragma pack(push, 1)
enum class XevtcEventSource : uint8_t
{
	Area = 0,
	Local = 1
};

struct XevtcString
{
	uint32_t Index; // Index of this string in the strings section of the evtc
};

struct XevtcHeader
{
	uint32_t Version;
	uint32_t StringCount;
	uint32_t EventCount;
};
#pragma pack(pop)

// This is not pragma packed to enable using it in-memory without a performance penalty
struct XevtcEvent
{
	struct : cbtevent
	{
		bool present;
	} ev;

	struct
	{
		uintptr_t id;
		Prof prof;
		uint32_t elite;
		uint32_t self;
		XevtcString name;
		uint16_t team;

		bool present;
	} source_ag;

	struct
	{
		uintptr_t id;
		Prof prof;
		uint32_t elite;
		uint32_t self;
		XevtcString name;
		uint16_t team;

		bool present;
	} destination_ag;

	XevtcEventSource collector_source;
	XevtcString skillname;
	uint64_t id;
	uint64_t revision;
};
static_assert(sizeof(XevtcEvent) == 160, "");

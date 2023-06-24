#include "GlobalObjects.h"

#include <Windows.h>

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		GlobalObjects::SELF_DLL = hModule;
		if (HANDLE mumbleFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink"); mumbleFileHandle) {
			LPVOID fileHandle = MapViewOfFile(mumbleFileHandle, FILE_MAP_READ, 0, 0, 0);
			GlobalObjects::MUMBLE_MEM = static_cast<LinkedMem*>(fileHandle);
		}
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

// This triggers the linker to pick up the two exported functions from the static library
void UnusedFunctionToHelpTheLinker()
{
	get_init_addr(nullptr, nullptr, nullptr, NULL, nullptr, nullptr, 0);
	get_release_addr();
}

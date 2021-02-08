/*
* arcdps combat api example
*/

#include <stdint.h>
#include <stdio.h>
#include <Windows.h>
#include <string>
#include <list>

#include "imgui\imgui.h"
#include "simpleini\SimpleIni.h"

#include "ArcdpsDataStructures.h"
#include "Player.h"
#include "Tracker.h"
#include "AppChart.h"
#include "Helpers.h"

/* proto/globals */
arcdps_exports arc_exports;
char* arcvers;
void dll_init(HANDLE hModule);
void dll_exit();
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dd9, HANDLE arcdll, void* mallocfn, void* freefn);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);
uintptr_t mod_imgui(uint32_t not_charsel_or_loading); /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
uintptr_t mod_options(); /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
void readArcExports();
void parseIni();
void writeIni();
bool modsPressed();
bool canMoveWindows();

Tracker tracker;

AppChart chart;
bool show_chart = false;

typedef uint64_t(*arc_export_func_u64)();

// load arcdps dll.
// When loading directly, arcdps is the "d3d9.dll"
// When loading with the addon manager, arcdps is called "gw2addon_arcdps.dll"
HMODULE arc_dllD = LoadLibraryA("d3d9.dll");
HMODULE arc_dllL = LoadLibraryA("gw2addon_arcdps.dll");
HMODULE arc_dll = (arc_dllL != nullptr) ? arc_dllL : arc_dllD;

// get exports
auto arc_export_e6 = (arc_export_func_u64)GetProcAddress(arc_dll, "e6");
auto arc_export_e7 = (arc_export_func_u64)GetProcAddress(arc_dll, "e7");

// arc globals
WPARAM arc_global_mod1;
WPARAM arc_global_mod2;
WPARAM arc_global_mod_multi;
bool arc_hide_all = false;
bool arc_panel_always_draw = false;
bool arc_movelock_altui = false;
bool arc_clicklock_altui = false;
bool arc_window_fastclose = false;

CSimpleIniA table_ini(true);
bool valid_table_ini = false;
WPARAM table_key;


/* dll main -- winapi */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
	switch (ulReasonForCall) {
	case DLL_PROCESS_ATTACH:
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return 1;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dd9, HANDLE arcdll, void* mallocfn, void* freefn) {
	arcvers = arcversionstr;
	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguicontext));
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

	parseIni();
	
	return mod_init;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_release_addr() {
	arcvers = 0;
	return mod_release;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init()
{
	/* for arcdps */
	memset(&arc_exports, 0, sizeof(arcdps_exports));
	arc_exports.sig = 0x64003268;//from random.org
	arc_exports.imguivers = IMGUI_VERSION_NUM;
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "Boon Table";
	arc_exports.out_build = __VERSION__;
	arc_exports.wnd_nofilter = mod_wnd;
	arc_exports.combat = mod_combat;
	arc_exports.imgui = mod_imgui;
	arc_exports.options_end = mod_options;
	return &arc_exports;
}

/* release mod -- return ignored */
uintptr_t mod_release()
{
	writeIni();
	return 0;
}

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	auto const io = &ImGui::GetIO();

	switch (uMsg)
	{
	case WM_KEYUP:
	{
		const int vkey = (int)wParam;
		io->KeysDown[vkey] = 0;
		if (vkey == VK_CONTROL)
		{
			io->KeyCtrl = false;
		}
		else if (vkey == VK_MENU)
		{
			io->KeyAlt = false;
		}
		else if (vkey == VK_SHIFT)
		{
			io->KeyShift = false;
		}
		break;
	}
	case WM_KEYDOWN:
	{
		const int vkey = (int)wParam;
		io->KeysDown[vkey] = 1;
		if (vkey == VK_CONTROL)
		{
			io->KeyCtrl = true;
		}
		else if (vkey == VK_MENU)
		{
			io->KeyAlt = true;
		}
		else if (vkey == VK_SHIFT)
		{
			io->KeyShift = true;
		}
		break;
	}
	case WM_SYSKEYUP:
	{
		const int vkey = (int)wParam;
		io->KeysDown[vkey] = 0;
		if (vkey == VK_CONTROL)
		{
			io->KeyCtrl = false;
		}
		else if (vkey == VK_MENU)
		{
			io->KeyAlt = false;
		}
		else if (vkey == VK_SHIFT)
		{
			io->KeyShift = false;
		}
		break;
	}
	case WM_SYSKEYDOWN:
	{
		const int vkey = (int)wParam;
		io->KeysDown[vkey] = 1;
		if (vkey == VK_CONTROL)
		{
			io->KeyCtrl = true;
		}
		else if (vkey == VK_MENU)
		{
			io->KeyAlt = true;
		}
		else if (vkey == VK_SHIFT)
		{
			io->KeyShift = true;
		}
		break;
	}
	case WM_ACTIVATEAPP:
	{
		if (!wParam)
		{
			io->KeysDown[arc_global_mod1] = false;
			io->KeysDown[arc_global_mod2] = false;
		}
		break;
	}
	break;
	}

	if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2])
	{
		if (io->KeysDown[table_key]) return 0;
	}
	return uMsg;
}

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision)
{
	Player* current_player = nullptr;

	/* ev is null. dst will only be valid on tracking add. skillname will also be null */
	if (!ev)
	{
		if (src)
		{
			/* notify tracking change */
			if (!src->elite)
			{
				/* add */
				if (src->prof)
				{
					if (dst)
					{
						tracker.addPlayer(src,dst);
					}
				}

				/* remove */
				else
				{
					tracker.removePlayer(src);
				}
			}

			/* notify target change */
			else if (src->elite == 1)
			{

			}
		}
	}

	/* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
	else
	{
		if(ev->time > 0) current_time = ev->time;

		/* statechange */
		if (ev->is_statechange)
		{
			if (ev->is_statechange == CBTS_ENTERCOMBAT)
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->combatEnter(ev);
					tracker.bakeCombatData();
				}
			}
			else if (ev->is_statechange == CBTS_EXITCOMBAT)
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->combatExit(ev);
				}
			}
		}

		/* activation */
		else if (ev->is_activation)
		{

		}

		/* buff remove */
		else if (ev->is_buffremove)
		{
			if (ev->is_buffremove == CBTB_MANUAL)//TODO: move to tracker
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->removeBoon(ev);
				}
			}
		}

		/* buff */
		else if (ev->buff)
		{

			/* damage */
			if (ev->buff_dmg)
			{
				
			}

			/* application */
			else
			{
				tracker.applyBoon(src, dst, ev);
			}
		}

		/* physical */
		else
		{
			
		}

		/* common */
	}
	return 0;
}

uintptr_t mod_imgui(uint32_t not_charsel_or_loading)
{
	readArcExports();
	
	if (!not_charsel_or_loading) return 0;

	auto io = &ImGui::GetIO();

	if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2])
	{
		if (ImGui::IsKeyPressed(table_key))
		{
			show_chart = !show_chart;
		}
	}

	if (show_chart)
	{
		chart.Draw("Boon Table", &show_chart, tracker, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | (!canMoveWindows() ? ImGuiWindowFlags_NoMove : 0));
	}
	return 0;
}
uintptr_t mod_options()
{
	ImGui::Checkbox("Boon Table", &show_chart);
	return 0;
}

void readArcExports()
{
	uint64_t e6_result = arc_export_e6();
	uint64_t e7_result = arc_export_e7();

	arc_hide_all = (e6_result & 0x01);
	arc_panel_always_draw = (e6_result & 0x02);
	arc_movelock_altui = (e6_result & 0x04);
	arc_clicklock_altui = (e6_result & 0x08);
	arc_window_fastclose = (e6_result & 0x10);


	uint16_t* ra = (uint16_t*)&e7_result;
	if (ra)
	{
		arc_global_mod1 = ra[0];
		arc_global_mod2 = ra[1];
		arc_global_mod_multi = ra[2];
	}
}

void parseIni()
{
	SI_Error rc = table_ini.LoadFile("addons\\arcdps\\arcdps_table.ini");
	valid_table_ini = rc >= 0;

	std::string pszValue = table_ini.GetValue("table", "show", "0");
	show_chart = std::stoi(pszValue);

	pszValue = table_ini.GetValue("table", "key", "66");
	table_key = std::stoi(pszValue);

	pszValue = table_ini.GetValue("table", "show_players", "1");
	chart.setShowPlayers(std::stoi(pszValue));

	pszValue = table_ini.GetValue("table", "show_subgroups", "1");
	chart.setShowSubgroups(std::stoi(pszValue));

	pszValue = table_ini.GetValue("table", "show_total", "1");
	chart.setShowTotal(std::stoi(pszValue));

	pszValue = table_ini.GetValue("table", "show_uptime_as_progress_bar", "1");
	chart.setShowBoonAsProgressBar(std::stoi(pszValue));

	for (auto boon_def = tracked_buffs.begin(); boon_def != tracked_buffs.end(); ++boon_def)
	{
		pszValue = table_ini.GetValue("boons", boon_def->name.c_str(), std::to_string(boon_def->is_relevant).c_str());
		boon_def->is_relevant = std::stoi(pszValue);
	}
}

void writeIni()
{
	SI_Error rc = table_ini.SetValue("table", "show", std::to_string(show_chart).c_str());

	rc = table_ini.SetValue("table", "show_players", std::to_string(chart.bShowPlayers(nullptr)).c_str());
	rc = table_ini.SetValue("table", "show_subgroups", std::to_string(chart.getShowSubgroups()).c_str());
	rc = table_ini.SetValue("table", "show_total", std::to_string(chart.bShowTotal()).c_str());
	rc = table_ini.SetValue("table", "show_uptime_as_progress_bar", std::to_string(chart.bShowBoonAsProgressBar()).c_str());

	for (auto boon_def = tracked_buffs.begin(); boon_def != tracked_buffs.end(); ++boon_def)
	{
		rc = table_ini.SetValue("boons", boon_def->name.c_str(), std::to_string(boon_def->is_relevant).c_str());
	}

	rc = table_ini.SaveFile("addons\\arcdps\\arcdps_table.ini");
}

bool modsPressed()
{
	auto io = &ImGui::GetIO();

	return io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2];
}

bool canMoveWindows()
{
	if (!arc_movelock_altui)
	{
		return true;
	}
	else
	{
		return modsPressed();
	}
}
/*
* arcdps combat api example
*/

#include <stdint.h>
#include <stdio.h>
#include <Windows.h>
#include <d3d9.h>
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
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);
uintptr_t mod_imgui(uint32_t not_charsel_or_loading); /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
uintptr_t mod_options(); /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
void parseIni();
void writeIni();
bool modsPressed();
bool canMoveWindows();

Tracker tracker;

AppChart chart;
bool show_chart = false;

CSimpleIniA arc_ini(true);
bool valid_arc_ini = false;
WPARAM arc_global_mod1;
WPARAM arc_global_mod2;
bool arc_movelock_altui = false;

CSimpleIniA table_ini(true);
bool valid_table_ini = false;
WPARAM table_key;


/* dll main -- winapi */
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
	switch (ulReasonForCall) {
	case DLL_PROCESS_ATTACH: dll_init(hModule); break;
	case DLL_PROCESS_DETACH: dll_exit(); break;

	case DLL_THREAD_ATTACH:  break;
	case DLL_THREAD_DETACH:  break;
	}
	return 1;
}

/* dll attach -- from winapi */
void dll_init(HANDLE hModule) {
	return;
}

/* dll detach -- from winapi */
void dll_exit() {
	return;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9) {
	arcvers = arcversionstr;
	ImGui::SetCurrentContext((ImGuiContext*)imguicontext);

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
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "Boon Table";
	arc_exports.out_build = "0.1";
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
					current_player->combatExit();
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
			if (ev->is_buffremove == CBTB_MANUAL)
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->removeBoon(ev);
					tracker.queueResort();
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
				if ((current_player = tracker.getPlayer(src->id)) && is_player(dst))
				{
					current_player->gaveBoon(ev);
					tracker.queueResort();
				}
				if (current_player = tracker.getPlayer(dst->id))
				{
					current_player->applyBoon(ev);
					tracker.queueResort();
				}
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
		tracker.sortPlayers();

		chart.Draw("Boon Table", &show_chart, &tracker, ImGuiWindowFlags_NoCollapse 
			| (!canMoveWindows() ? ImGuiWindowFlags_NoMove : 0));
	}
	return 0;
}
uintptr_t mod_options()
{
	ImGui::Checkbox("Boon Table", &show_chart);
	return 0;
}

void parseIni()
{
	SI_Error rc = arc_ini.LoadFile("addons\\arcdps\\arcdps.ini");
	valid_arc_ini = rc < 0;

	std::string pszValue = arc_ini.GetValue("keys", "global_mod1", "0x10");
	arc_global_mod1 = std::stoi(pszValue, 0, 16);

	pszValue = arc_ini.GetValue("keys", "global_mod2", "0x12");
	arc_global_mod2 = std::stoi(pszValue, 0, 16);

	pszValue = arc_ini.GetValue("session", "movelock_altui", "0");
	arc_movelock_altui = std::stoi(pszValue);

	rc = table_ini.LoadFile("addons\\arcdps\\arcdps_table.ini");
	valid_table_ini = rc < 0;

	pszValue = table_ini.GetValue("table", "show", "0");
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

	pszValue = table_ini.GetValue("table", "table_to_display", "0");
	tracker.table_to_display = std::stoi(pszValue);

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
	rc = table_ini.SetValue("table", "show_subgroups", std::to_string(chart.bShowSubgroups(nullptr)).c_str());
	rc = table_ini.SetValue("table", "show_total", std::to_string(chart.bShowTotal(nullptr)).c_str());
	rc = table_ini.SetValue("table", "show_uptime_as_progress_bar", std::to_string(chart.bShowBoonAsProgressBar()).c_str());
	rc = table_ini.SetValue("table", "table_to_display", std::to_string(tracker.table_to_display).c_str());

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
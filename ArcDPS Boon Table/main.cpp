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
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname);
uintptr_t mod_imgui(); /* id3dd9::present callback, before imgui::render, fn() */
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
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext) {
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
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "Boon Table";
	arc_exports.out_build = "0.1";
	arc_exports.sig = 0x64003268;//from random.org
	arc_exports.wnd = mod_wnd;
	arc_exports.combat = mod_combat;
	arc_exports.imgui = mod_imgui;
	arc_exports.options = mod_options;
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
	auto io = &ImGui::GetIO();

	if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2])
	{
		if (io->KeysDown[table_key]) return 0;
	}
	return uMsg;
}

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname)
{
	Player* current_player = nullptr;

	/* ev is null. dst will only be valid on tracking add. skillname will also be null */
	if (!ev)
	{

		/* notify tracking change */
		if (!src->elite)
		{

			/* add */
			if (src->prof)
			{
				tracker.addPlayer(src->id, std::string(src->name));
			}

			/* remove */
			else
			{
				tracker.removePlayer(src->id);
			}
		}

		/* notify target change */
		else if (src->elite == 1)
		{
			
		}
	}

	/* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
	else
	{
		/* statechange */
		if (ev->is_statechange)
		{
			if (ev->is_statechange == CBTS_ENTERCOMBAT)
			{
				if (current_player = tracker.getPlayer(src))
				{
					current_player->combatEnter(getCurrentTime(),ev->dst_agent);
				}
			}
			else if (ev->is_statechange == CBTS_EXITCOMBAT)
			{
				if (current_player = tracker.getPlayer(src))
				{
					current_player->combatExit(getCurrentTime());
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
			if (current_player = tracker.getPlayer(src))
			{
//				current_player->removeBoon(ev);
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
				if (current_player = tracker.getPlayer(dst))
				{
					current_player->applyBoon(ev);
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

uintptr_t mod_imgui()
{
	auto io = &ImGui::GetIO();

	if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2])
	{
		if (ImGui::IsKeyPressed(table_key))
		{
			show_chart = !show_chart;
		}
	}

	if (show_chart) chart.Draw("BOON TABLE", &show_chart, &tracker, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar 
		| (!canMoveWindows() ? ImGuiWindowFlags_NoMove : 0));
	return 0;
}
uintptr_t mod_options()
{
	ImGui::Checkbox("BOON TABLE", &show_chart);
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

	pszValue = table_ini.GetValue("table", "key", "86");
	table_key = std::stoi(pszValue);

	for (std::list<BoonDef>::iterator boon_def = tracked_buffs.begin(); boon_def != tracked_buffs.end(); ++boon_def)
	{
		pszValue = table_ini.GetValue("boons", boon_def->name.c_str(), std::to_string(boon_def->is_relevant).c_str());
		boon_def->is_relevant = std::stoi(pszValue);
	}
}

void writeIni()
{
	SI_Error rc = table_ini.SetValue("table", "show", std::to_string(show_chart).c_str());

	for (std::list<BoonDef>::iterator boon_def = tracked_buffs.begin(); boon_def != tracked_buffs.end(); ++boon_def)
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
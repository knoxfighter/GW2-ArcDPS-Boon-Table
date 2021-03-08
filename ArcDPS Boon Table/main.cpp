/*
* arcdps combat api example
*/

#include <cstdint>
#include <Windows.h>
#include <string>
#include <iostream>

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
uint32_t cbtcount = 0;
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dd9, HMODULE arcdll, void* mallocfn, void* freefn);
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

void log_file(char* str);
void log_arc(char* str);

/* arcdps exports */
void* filelog;
void* arclog;

/* log to arcdps.log, thread/async safe */
void log_file(char* str) {
	size_t(*log)(char*) = (size_t(*)(char*))filelog;
	if (log) (*log)(str);
	return;
}
/* log to extensions tab in arcdps log window, thread/async safe */
void log_arc(char* str) {
	size_t(*log)(char*) = (size_t(*)(char*))arclog;
	if (log) (*log)(str);
	return;
}

Tracker tracker;

AppChart chart;
bool show_chart = false;

typedef uint64_t(*arc_export_func_u64)();
typedef void(*log_func)(char* str);

HMODULE arc_dll;

// get exports
arc_color_func arc_export_e5;
arc_export_func_u64 arc_export_e6;
arc_export_func_u64 arc_export_e7;

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
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dd9, HMODULE new_arcdll, void* mallocfn, void* freefn) {
	// set all arcdps stuff
	arcvers = arcversionstr;
	arc_dll = new_arcdll;
	arc_export_e5 = (arc_color_func)GetProcAddress(arc_dll, "e5");
	arc_export_e6 = (arc_export_func_u64)GetProcAddress(arc_dll, "e6");
	arc_export_e7 = (arc_export_func_u64)GetProcAddress(arc_dll, "e7");
	filelog = (void*)GetProcAddress((HMODULE)arc_dll, "e3");
	arclog = (void*)GetProcAddress((HMODULE)arc_dll, "e8");

	// set imgui context && allocation for arcdps dll space
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
	log_arc((char*)"boon-table: done mod_init"); // if using vs2015+, project properties > c++ > conformance mode > permissive to avoid const to not const conversion error
	log_file((char*)"boon-table: done mod init");
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
	case WM_KEYDOWN:
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
	}

	if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2])
	{
		if (io->KeysDown[table_key]) return 0;
	}
	return uMsg;
}

bool addedGlenna = false;
uintptr_t glenna_id;


/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision)
{
	Player* current_player = nullptr;
	char buff[4096];
	char* p = &buff[0];

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
					p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
					p += _snprintf_s(p, 400, _TRUNCATE, "agent added: %s:%s (%0llx), instid: %u, prof: %u, elite: %u, self: %u, team: %u, subgroup: %u\n", src->name, dst->name, src->id, dst->id, dst->prof, dst->elite, dst->self, src->team, dst->team);
					
					if (dst || dst->name)
					{
						tracker.addPlayer(src,dst);
						chart.needSort = true;
					}
				}

				/* remove */
				else
				{
					p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
					p += _snprintf_s(p, 400, _TRUNCATE, "agent removed: %s (%0llx)\n", src->name, src->id);
					tracker.removePlayer(src);
				}
			}

			/* notify target change */
			else if (src->elite == 1)
			{
				p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
				p += _snprintf_s(p, 400, _TRUNCATE, "new target: %0llx\n", src->id);

			}
		}
	}

	/* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
	else
	{
		/* default names */
		if (!src->name || !strlen(src->name)) src->name = (char*)"(area)";
		if (!dst->name || !strlen(dst->name)) dst->name = (char*)"(area)";
		
		/* common */
		p += _snprintf_s(p, 400, _TRUNCATE, "combatdemo: ==== cbtevent %u at %llu ====\n", cbtcount, ev->time);
		p += _snprintf_s(p, 400, _TRUNCATE, "source agent: %s (%0llx:%u, %lx:%lx), master: %u; %u\n", src->name, ev->src_agent, ev->src_instid, src->prof, src->elite, ev->src_master_instid, src->id);
		if (ev->dst_agent) p += _snprintf_s(p, 400, _TRUNCATE, "target agent: %s (%0llx:%u, %lx:%lx); %u\n", dst->name, ev->dst_agent, ev->dst_instid, dst->prof, dst->elite, dst->id);
		else p += _snprintf_s(p, 400, _TRUNCATE, "target agent: n/a\n");

		std::string name_glenna = "Priory Scholar";

		if (!addedGlenna && name_glenna.compare(dst->name) == 0) {
			p += _snprintf_s(p, 400, _TRUNCATE, "Added Priory Scholar\n");
			addedGlenna = true;
			tracker.addPlayer(dst->id,
				1,
				PROF_UNKNOWN,
				dst->name,
				"NPC"
			);
			glenna_id = dst->id;
		}

		if(ev->time > 0) current_time = ev->time;

		/* statechange */
		if (ev->is_statechange)
		{
			p += _snprintf_s(p, 400, _TRUNCATE, "is_statechange: %u\n", ev->is_statechange);
			if (ev->is_statechange == CBTS_ENTERCOMBAT)
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->combatEnter(ev);
					tracker.bakeCombatData();
					chart.needSort = true;

					if(src->self) {
						if (current_player = tracker.getPlayer(glenna_id))
						{
							current_player->combatEnter(ev);
							tracker.bakeCombatData();
							chart.needSort = true;
						}
					}
				}
			}
			else if (ev->is_statechange == CBTS_EXITCOMBAT)
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->combatExit(ev);

					if (src->self) {
						if (current_player = tracker.getPlayer(glenna_id))
						{
							current_player->combatExit(ev);
						}
					}
				}
			}
		}

		/* activation */
		else if (ev->is_activation)
		{
			p += _snprintf_s(p, 400, _TRUNCATE, "is_activation: %u\n", ev->is_activation);
			p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf_s(p, 400, _TRUNCATE, "ms_expected: %d\n", ev->value);

		}

		/* buff remove */
		else if (ev->is_buffremove)
		{
			p += _snprintf_s(p, 400, _TRUNCATE, "is_buffremove: %u\n", ev->is_buffremove);
			p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf_s(p, 400, _TRUNCATE, "ms_duration: %d\n", ev->value);
			p += _snprintf_s(p, 400, _TRUNCATE, "ms_intensity: %d\n", ev->buff_dmg);
			if (ev->is_buffremove == CBTB_MANUAL)//TODO: move to tracker
			{
				if (current_player = tracker.getPlayer(src->id))
				{
					current_player->removeBoon(ev);
					chart.needSort = true;
				}
			}
		}

		/* buff */
		else if (ev->buff)
		{

			/* damage */
			if (ev->buff_dmg)
			{

				p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
				p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
				p += _snprintf_s(p, 400, _TRUNCATE, "dmg: %d\n", ev->buff_dmg);
				p += _snprintf_s(p, 400, _TRUNCATE, "is_shields: %u\n", ev->is_shields);
			}

			/* application */
			else
			{
				p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
				p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
				p += _snprintf_s(p, 400, _TRUNCATE, "raw ms: %d\n", ev->value);
				p += _snprintf_s(p, 400, _TRUNCATE, "overstack ms: %u\n", ev->overstack_value);

				tracker.applyBoon(src, dst, ev);
				chart.needSort = true;
			}
		}

		/* physical */
		else
		{
			p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
			p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf_s(p, 400, _TRUNCATE, "dmg: %d\n", ev->value);
			p += _snprintf_s(p, 400, _TRUNCATE, "is_moving: %u\n", ev->is_moving);
			p += _snprintf_s(p, 400, _TRUNCATE, "is_ninety: %u\n", ev->is_ninety);
			p += _snprintf_s(p, 400, _TRUNCATE, "is_flanking: %u\n", ev->is_flanking);
			p += _snprintf_s(p, 400, _TRUNCATE, "is_shields: %u\n", ev->is_shields);
			
		}

		/* common */
		p += _snprintf_s(p, 400, _TRUNCATE, "iff: %u\n", ev->iff);
		p += _snprintf_s(p, 400, _TRUNCATE, "result: %u\n", ev->result);
		cbtcount += 1;
	}
	log_arc(&buff[0]);
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
		chart.Draw(&show_chart, tracker, !canMoveWindows() ? ImGuiWindowFlags_NoMove : 0);
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

	std::string pszValueString = table_ini.GetValue("table", "show", "0");
	show_chart = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "key", "66");
	table_key = std::stoi(pszValueString);

	pszValueString = table_ini.GetValue("table", "show_players", "1");
	chart.setShowPlayers(std::stoi(pszValueString));

	pszValueString = table_ini.GetValue("table", "show_subgroups", "1");
	chart.setShowSubgroups(std::stoi(pszValueString));

	pszValueString = table_ini.GetValue("table", "show_total", "1");
	chart.setShowTotal(std::stoi(pszValueString));

	pszValueString = table_ini.GetValue("table", "show_uptime_as_progress_bar", "1");
	chart.setShowBoonAsProgressBar(std::stoi(pszValueString));

	pszValueString = table_ini.GetValue("table", "show_colored", "0");
	long show_colored = table_ini.GetLongValue("table", "show_colored", static_cast<long>(ProgressBarColoringMode::Uncolored));
	chart.setShowColored(static_cast<ProgressBarColoringMode>(show_colored));

	bool size_to_content = table_ini.GetBoolValue("table", "size_to_content", true);
	chart.setSizeToContent(size_to_content);

	bool alternating_row_bg = table_ini.GetBoolValue("table", "alternating_row_bg", true);
	chart.setAlternatingRowBg(alternating_row_bg);

	long pszValueLong = table_ini.GetLongValue("table", "alignment", static_cast<long>(Alignment::Right));
	chart.setAlignment(static_cast<Alignment>(pszValueLong));
}

void writeIni()
{
	SI_Error rc = table_ini.SetValue("table", "show", std::to_string(show_chart).c_str());

	rc = table_ini.SetValue("table", "show_players", std::to_string(chart.bShowPlayers()).c_str());
	rc = table_ini.SetValue("table", "show_subgroups", std::to_string(chart.getShowSubgroups()).c_str());
	rc = table_ini.SetValue("table", "show_total", std::to_string(chart.bShowTotal()).c_str());
	rc = table_ini.SetValue("table", "show_uptime_as_progress_bar", std::to_string(chart.bShowBoonAsProgressBar()).c_str());
	rc = table_ini.SetLongValue("table", "show_colored", static_cast<long>(chart.getShowColored()));
	rc = table_ini.SetBoolValue("table", "size_to_content", chart.bSizeToContent());
	rc = table_ini.SetBoolValue("table", "alternating_row_bg", chart.bAlternatingRowBg());
	rc = table_ini.SetLongValue("table", "alignment", static_cast<long>(chart.getAlignment()));

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
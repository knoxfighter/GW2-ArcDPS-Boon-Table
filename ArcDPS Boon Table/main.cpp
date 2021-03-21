/*
* arcdps combat api example
*/

#include <cstdint>
#include <Windows.h>
#include <string>
#include <regex>
#include <d3d9.h>

#include "imgui/imgui.h"
#include "simpleini/SimpleIni.h"
#include "extension/arcdps_structs.h"
#include "Player.h"
#include "Tracker.h"
#include "AppChart.h"
#include "Helpers.h"
#include "Lang.h"
#include "Settings.h"
#include "SettingsUI.h"

/* proto/globals */
arcdps_exports arc_exports;
char* arcvers;
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9, HMODULE arcdll, void* mallocfn, void* freefn);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision);
uintptr_t mod_imgui(uint32_t not_charsel_or_loading); /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
uintptr_t mod_options(); /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
void readArcExports();
bool modsPressed();
bool canMoveWindows();

Tracker tracker;
AppChart chart;
SettingsUI settingsUi;

typedef uint64_t(*arc_export_func_u64)();
typedef void(*log_func)(char* str);

HMODULE arc_dll;
HMODULE self_dll;

// get exports
arc_color_func arc_export_e5;
arc_export_func_u64 arc_export_e6;
arc_export_func_u64 arc_export_e7;
arc_log_func_ptr arc_log_file;
arc_log_func_ptr arc_log;

// arc globals
WPARAM arc_global_mod1;
WPARAM arc_global_mod2;
WPARAM arc_global_mod_multi;
bool arc_hide_all = false;
bool arc_panel_always_draw = false;
bool arc_movelock_altui = false;
bool arc_clicklock_altui = false;
bool arc_window_fastclose = false;

/* dll main -- winapi */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID lpReserved) {
	switch (ulReasonForCall) {
	case DLL_PROCESS_ATTACH:
		self_dll = hModule;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return 1;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, IDirect3DDevice9* id3dd9, HMODULE new_arcdll, void* mallocfn, void* freefn) {
	// set all arcdps stuff
	arcvers = arcversionstr;
	arc_dll = new_arcdll;
	arc_export_e5 = (arc_color_func)GetProcAddress(arc_dll, "e5");
	arc_export_e6 = (arc_export_func_u64)GetProcAddress(arc_dll, "e6");
	arc_export_e7 = (arc_export_func_u64)GetProcAddress(arc_dll, "e7");
	arc_log_file = (arc_log_func_ptr)GetProcAddress(arc_dll, "e3");
	arc_log = (arc_log_func_ptr)GetProcAddress(arc_dll, "e8");

	// set imgui context && allocation for arcdps dll space
	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguicontext));
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

	init_tracked_buffs(id3dd9);
	
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
		if (io->KeysDown[settings.getTableKey()]) return 0;
	}
	return uMsg;
}

constexpr auto num_of_npcs = 3;

//TODO: Figure something better out, maybe some id, like for bosses, is shown somewhere?
const std::regex npc_names[3] = {
	std::basic_regex("(Priory Scholar)|(Abtei-Gelehrte)|(Erudite du Prieur.*)|(Erudita del Priorato)"), //Glenna (Wing 3, 1st encounter: Escort)
	std::basic_regex("(Saul.*)"), //Saul D'Alessio (Wing 4, 4th encounter: Deimos)
	std::basic_regex("(Desmina)") //Desmina, River of Souls
};

uintptr_t npc_ids[num_of_npcs];

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision)
{
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
					if (dst && dst->name)
					{
						tracker.addPlayer(src,dst);
						chart.needSort = true;
					}
				}

				/* remove */
				else
				{
					tracker.removePlayer(src);

					//src->self is not set here
					if (src->id == 2000) {
						//all npcs removed with the player (i.e. swapping wings)
						tracker.clearNPCs();
					}
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
		/* common */

		
		for (int i = 0; i < num_of_npcs; i++) {
			//Player
			// Self: 200
			// Own Illusion: 200
			// Other players: 200
			//Friendly
			// Glenna: 194
			// w7 Djins: 194
			// Priory Arcanist: 194
			// Saul: 194
			//Enemy:
			// w7 trash: 199
			// undead eagle in orr: 263
			if (dst->team == 194 && std::regex_match(dst->name, npc_names[i])) {
				npc_ids[i] = dst->id;
				tracker.addNPC(dst->id, dst->name, ev);
			}
		}

		if(ev->time > 0) current_time = ev->time;

		/* statechange */
		if (ev->is_statechange)
		{
			Player* player = tracker.getPlayer(src->id);
			if (ev->is_statechange == CBTS_ENTERCOMBAT)
			{
				if (player = tracker.getPlayer(src->id))
				{
					player->combatEnter(ev);

					if(src->self)
					{

						for (std::list<NPC>::iterator it = tracker.npcs.begin(); it != tracker.npcs.end(); ++it)
						{
							it->combatEnter(ev);
						}
					}
					tracker.bakeCombatData();
					chart.needSort = true;
				}
			}
			else if (ev->is_statechange == CBTS_EXITCOMBAT)
			{
				if (player)
				{
					player->combatExit(ev);

					if (src->self)
					{
						for (std::list<NPC>::iterator it = tracker.npcs.begin(); it != tracker.npcs.end(); ++it)
						{
							it->combatExit(ev);
						}
					}
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
				Entity* entity = tracker.getEntity(src->id);
				if (entity)
				{
					entity->removeBoon(ev);
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

			}

			/* application */
			else
			{
				tracker.applyBoon(src, dst, ev);
				chart.needSort = true;
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
		if (ImGui::IsKeyPressed(settings.getTableKey()))
		{
			settings.show_chart = !settings.show_chart;
		}
	}

	if (settings.show_chart)
	{
		chart.Draw(&settings.show_chart, tracker, !canMoveWindows() ? ImGuiWindowFlags_NoMove : 0);
	}
	return 0;
}
uintptr_t mod_options()
{
	ImGui::Checkbox(lang.translate(LangKey::ShowChart).c_str(), &settings.show_chart);
	ImGui::SameLine();
	ImGui::BeginChild("boonTableSettings", ImVec2(0, ImGui::GetTextLineHeight()));
	if (ImGui::BeginMenu("##boon-table-settings")) {
		settingsUi.Draw();
		ImGui::EndMenu();
	}
	ImGui::EndChild();
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
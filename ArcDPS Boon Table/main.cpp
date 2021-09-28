/*
* arcdps combat api example
*/

#include <cstdint>
#include <Windows.h>
#include <string>
#include <regex>
#include <d3d11.h>
#include <d3d9.h>

#include "imgui/imgui.h"
#include "extension/arcdps_structs.h"
#include "Player.h"
#include "Tracker.h"
#include "AppChart.h"
#include "Helpers.h"
#include "History.h"
#include "Lang.h"
#include "Settings.h"
#include "SettingsUIGlobal.h"
#include "UpdateChecker.h"
#include "extension/MumbleLink.h"
#include "extension/Widgets.h"
#include "imgui/imgui_internal.h"

/* proto/globals */
arcdps_exports arc_exports{};
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* dxptr, HMODULE arcdll, void* mallocfn, void* freefn, UINT dxVer);
extern "C" __declspec(dllexport) void* get_release_addr();
arcdps_exports* mod_init();
uintptr_t mod_release();
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision);
uintptr_t mod_imgui(uint32_t not_charsel_or_loading); /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
uintptr_t mod_options(); /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
uintptr_t mod_options_windows(const char* windowname); // fn(char* windowname) 
void readArcExports();
bool modsPressed();
bool canMoveWindows();

typedef uint64_t(*arc_export_func_u64)();

HMODULE arc_dll;
HMODULE self_dll;
LPVOID mapViewOfMumbleFile = nullptr;
UINT directxVersion;
IDirect3DDevice9* id3dd9 = nullptr;
ID3D11Device* id3d11d = nullptr;

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
		if (HANDLE mumbleFileHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(LinkedMem), L"MumbleLink"); mumbleFileHandle) {
			mapViewOfMumbleFile = MapViewOfFile(mumbleFileHandle, FILE_MAP_READ, 0, 0, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return 1;
}

// (*fg)(m_version, ImGui::GetCurrentContext(), g_directx == 9 ? (void*)m_gl_pmyIDirect3DDevice9 : m_gl_d3d11swapchain, g_hthisinstance, malloc, free, g_directx);
/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* dxptr, HMODULE new_arcdll, void* mallocfn, void* freefn, UINT dxver) {
	// set all arcdps stuff
	arc_dll = new_arcdll;
	arc_export_e5 = (arc_color_func)GetProcAddress(arc_dll, "e5");
	arc_export_e6 = (arc_export_func_u64)GetProcAddress(arc_dll, "e6");
	arc_export_e7 = (arc_export_func_u64)GetProcAddress(arc_dll, "e7");
	arc_log_file = (arc_log_func_ptr)GetProcAddress(arc_dll, "e3");
	arc_log = (arc_log_func_ptr)GetProcAddress(arc_dll, "e8");

	PRINT_LINE()

	// set imgui context && allocation for arcdps dll space
	ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguicontext));
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

	// dx11 not available in older arcdps versions
	std::string arcVersion = arcversionstr;
	auto firstDot = arcVersion.find_first_of(".");
	arcVersion = arcVersion.substr(0, firstDot);
	int arcVersionNum = std::stoi(arcVersion);

	if (arcVersionNum > 20210828 && dxver == 11) {
		auto swapChain = static_cast<IDXGISwapChain*>(dxptr);
		swapChain->GetDevice(__uuidof(id3d11d), reinterpret_cast<void**>(&id3d11d));
		directxVersion = 11;
	} else {
		id3dd9 = static_cast<IDirect3DDevice9*>(dxptr);
		directxVersion = 9;
	} 

	return mod_init;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_release_addr() {
	return mod_release;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init()
{
	PRINT_LINE()
	bool loading_successful = true;
	std::string error_message = "Unknown error";
	
	try {
		// load settings
		settings.readFromFile();

		// load translation
		lang.readFromFile();

		// init buffs, this will load the icons into RAM
		init_tracked_buffs();

		UpdateCheckerBase::ClearFiles(self_dll);

		// check for new version on github
		const auto& version = UpdateCheckerBase::GetCurrentVersion(self_dll);
		if (version) {
			updateChecker.CheckForUpdate(version.value(), "knoxfighter/GW2-ArcDPS-Boon-Table", false);
		}

		// load my table loader into imgui
		ImGuiEx::BigTable::RegisterSettingsHandler("BigTable-BoonTable");

		// setup icon loader
		iconLoader.Setup(self_dll, id3dd9, id3d11d);
	} catch (std::exception& e) {
		loading_successful = false;
		error_message = "Error starting up: ";
		error_message.append(e.what());
	}
	
	/* for arcdps */
	arc_exports.imguivers = IMGUI_VERSION_NUM;
	arc_exports.out_name = "Boon Table";

	const std::string& version = UpdateCheckerBase::GetVersionAsString(self_dll);
    char* version_c_str = new char[version.length() + 1];
    strcpy_s(version_c_str, version.length() + 1, version.c_str());
	arc_exports.out_build = version_c_str;

	if (loading_successful) {
		arc_exports.size = sizeof(arcdps_exports);
		arc_exports.sig = 0x64003268;//from random.org
		arc_exports.wnd_nofilter = mod_wnd;
		arc_exports.combat = mod_combat;
		arc_exports.imgui = mod_imgui;
		arc_exports.options_end = mod_options;
		arc_exports.options_windows = mod_options_windows;
	} else {
		arc_exports.sig = 0;
		const std::string::size_type size = error_message.size();
		char* buffer = new char[size + 1]; //we need extra char for NUL
		memcpy(buffer, error_message.c_str(), size + 1);
		arc_exports.size = (uintptr_t)buffer;
	}
	return &arc_exports;
}

/* release mod -- return ignored */
uintptr_t mod_release()
{
	try {
		settings.saveToFile();

		lang.saveToFile();
	} catch(const std::exception& e) {
		arc_log_file("error in mod_release!");
		arc_log_file(e.what());
	}

	return 0;
}

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	try {
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
			for (const auto& shortcut : settings.getShortcuts()) {
				if (io->KeysDown[shortcut]) return 0;
			}
		}
	} catch (const std::exception& e) {
		arc_log_file("Boon Table: exception in mod_wnd");
		arc_log_file(e.what());
		throw e;
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
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) {
	PRINT_LINE()
	try {
		/* ev is null. dst will only be valid on tracking add. skillname will also be null */
		history.Event(dst);
		
		if (!ev) {
			if (src) {
				/* notify tracking change */
				if (!src->elite) {
					/* add */
					if (src->prof) {
						if (dst && dst->name) {
							liveTracker.addPlayer(src,dst);
							charts.sortNeeded();

							if (dst->self) {
								// this is yourself, check your map and see active "one-logging" if it is WvW
								if (mapViewOfMumbleFile) {
									LinkedMem* linkedMem = static_cast<LinkedMem*>(mapViewOfMumbleFile);
									constexpr std::array<uint32_t, 4> mapIDs {38, 95, 96, 1099};
									uint32_t mapId = linkedMem->getMumbleContext()->mapId;
									const auto& mapIdIt = std::find(mapIDs.begin(), mapIDs.end(), mapId);
									isWvW = mapIdIt != mapIDs.end();
								}
							}
						}
					}
					/* remove */
					else {
						liveTracker.removePlayer(src);
						charts.sortNeeded();
					}
				}
				/* notify target change */
				else if (src->elite == 1) {

				}
			}
		}
		/* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
		else {
			/* common */

			/*
			This code crashes, cause dst->name is not checked.
			Also the npc tracking is completely broken and needs deeper fixing and investigation!

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
			*/

			if(ev->time > 0) current_time = ev->time;

			/* statechange */
			if (ev->is_statechange) {
				if (ev->is_statechange == CBTS_ENTERCOMBAT) {
					Player* player = liveTracker.getPlayer(src->id);
					if (player) {
						player->combatEnter(ev);

						// FIXME: NPC stuff
						// if(src->self) {
						// 	for (NPC& npc : tracker.npcs) {
						// 		npc.combatEnter(ev);
						// 	}
						// }
						charts.sortNeeded();
					}
				}
				else if (ev->is_statechange == CBTS_EXITCOMBAT) {
					Player* player = liveTracker.getPlayer(src->id);
					if (player)
					{
						player->combatExit(ev);

						// FIXME: NPC stuff
						// if (src->self)
						// {
						// 	for (NPC& npc : tracker.npcs) {
						// 		npc.combatExit(ev);
						// 	}
						// }
					}
				}
				else if (ev->is_statechange == CBTS_STATRESET) {
					history.Reset(ev);
					
					std::lock_guard<std::mutex> guard(liveTracker.players_mtx);
					for (auto& pair : liveTracker.getPlayers()) {
						Player& player = pair.second;
						player.combatExit(ev);
						// do not call combatEnter on Player, cause ev->dst_agent (subgroup) is not set
						player.Entity::combatEnter(ev);
					}
				} else if (ev->is_statechange == CBTS_LOGSTART) {
					history.LogStart(ev);
				} else if (ev->is_statechange == CBTS_LOGEND) {
					history.LogEnd(ev);
				}
			}
			/* activation */
			else if (ev->is_activation) {
			}

			/* buff remove */
			else if (ev->is_buffremove) {
				if (ev->is_buffremove == CBTB_MANUAL) { //TODO: move to tracker
					Entity* entity = liveTracker.getEntity(src->id);
					if (entity) {
						entity->removeBoon(ev);
						charts.sortNeeded();
					}
				}
			}
			/* buff */
			else if (ev->buff) {
				/* damage */
				if (ev->buff_dmg) {

				}
				/* application */
				else {
					liveTracker.applyBoon(src, dst, ev);
					charts.sortNeeded();
				}
			}
			/* physical */
			else {
				// read out if players are above 90% health
				liveTracker.dealtDamage(src, ev);
			}

			/* common */
		}
	} catch(const std::exception& e) {
		arc_log_file("Boon Table: exception in mod_combat");
		arc_log_file(e.what());
		throw e;
	}
	return 0;
}

uintptr_t mod_imgui(uint32_t not_charsel_or_loading)
{
	PRINT_LINE()
	try {
		// ImGui::ShowDemoWindow();
		
		readArcExports();

		if (!not_charsel_or_loading) return 0;

		PRINT_LINE()

		auto io = &ImGui::GetIO();

		if (io->KeysDown[arc_global_mod1] && io->KeysDown[arc_global_mod2]) {
			const auto& shortcuts = settings.getShortcuts();
			for (size_t i = 0; i < MaxTableWindowAmount; ++i) {
				if (ImGui::IsKeyPressed(shortcuts[i])) {
					settings.setShowChart(i, !settings.isShowChart(i));
				}
			}
		}

		charts.drawAll(!canMoveWindows() ? ImGuiWindowFlags_NoMove : 0);

		updateChecker.Draw();
	} catch(const std::exception& e) {
		arc_log_file("Boon Table: exception in mod_imgui");
		arc_log_file(e.what());
		throw e;
	}
	return 0;
}

uintptr_t mod_options()
{
	try {
		settingsUiGlobal.Draw();
	} catch(const std::exception& e) {
		arc_log_file("Boon Table: exception in mod_options");
		arc_log_file(e.what());
		throw e;
	}
	return 0;
}

/**
 * @return true to disable this option
 */
uintptr_t mod_options_windows(const char* windowname) {
	try {
		if (!windowname) {
			ImGui::Checkbox(lang.translate(LangKey::ShowChart).c_str(), &settings.isShowChart(0));

			for (int i = 1; i < MaxTableWindowAmount; ++i) {
				std::string str = settings.getAppearAsInOption(i);
				if (str.empty()) {
					str = lang.translate(LangKey::ShowChart);
					str.append(" ");
					str.append(std::to_string(i));
				}
				ImGui::Checkbox(str.c_str(), &settings.isShowChart(i));
			}
		}
	} catch(const std::exception& e) {
		arc_log_file("Boon Table: exception in mod_options_windows");
		arc_log_file(e.what());
		throw e;
	}
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
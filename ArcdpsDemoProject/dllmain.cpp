#include "EventWindow.h"
#include "global.h"

#include "extension/arcdps_structs.h"
#include "extension/KeyBindHandler.h"
#include "extension/Windows/Demo/DemoTableWindow.h"
#include "extension/Windows/Demo/DemoWindow.h"
#include "extension/KeyInput.h"
#include "extension/windows/PositioningComponent.h"

#include "imgui/imgui.h"

#include <d3d11.h>
#include <d3d9.h>
#include <string>
#include <Windows.h>
#include <format>

namespace {
	HMODULE SELF_DLL;
	HMODULE ARC_DLL;
	IDirect3DDevice9* d3d9Device = nullptr;
	ID3D11Device* d3d11Device = nullptr;
	arcdps_exports arc_exports = {};
}

BOOL APIENTRY DllMain(HMODULE pModule,
					  DWORD pReasonForCall,
					  LPVOID pReserved
) {
	switch (pReasonForCall) {
		case DLL_PROCESS_ATTACH:
			SELF_DLL = pModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

uintptr_t mod_windows(const char* windowname) {
	if (!windowname) {
		EventWindow::instance().DrawOptionCheckbox();
#if _DEBUG
		DemoWindow::instance().DrawOptionCheckbox();
		DemoTableWindow::instance().DrawOptionCheckbox();
#endif
	}
	return 0;
}

uintptr_t mod_imgui(uint32_t not_charsel_or_loading) {
#if PERFORMANCE_LOG
	const auto& beforePoint = std::chrono::high_resolution_clock::now();
#endif

	EventWindow::instance().Draw();
#if _DEBUG
	DemoWindow::instance().Draw();
	DemoTableWindow::instance().Draw();
#endif

	if (not_charsel_or_loading) {
		/* Draw Windows here */
	}

#if PERFORMANCE_LOG
	const auto& afterPoint = std::chrono::high_resolution_clock::now();
	const auto& diff = afterPoint - beforePoint;
	ARC_LOG_FILE(std::format("mod_imgui: {}", diff.count()).c_str());
#endif

	return 0;
}

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#if PERFORMANCE_LOG
	const auto& beforePoint = std::chrono::high_resolution_clock::now();
#endif

	try {
		if (ImGuiEx::KeyCodeInputWndHandle(hWnd, uMsg, wParam, lParam)) {
			return 0;
		}

		if (KeyBindHandler::instance().Wnd(hWnd, uMsg, wParam, lParam)) {
			return 0;
		}

		auto const io = &ImGui::GetIO();

		switch (uMsg) {
			case WM_KEYUP:
			case WM_SYSKEYUP: {
				const int vkey = (int)wParam;
				io->KeysDown[vkey] = false;
				if (vkey == VK_CONTROL) {
					io->KeyCtrl = false;
				} else if (vkey == VK_MENU) {
					io->KeyAlt = false;
				} else if (vkey == VK_SHIFT) {
					io->KeyShift = false;
				}
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN: {
				const int vkey = (int)wParam;
				if (vkey == VK_CONTROL) {
					io->KeyCtrl = true;
				} else if (vkey == VK_MENU) {
					io->KeyAlt = true;
				} else if (vkey == VK_SHIFT) {
					io->KeyShift = true;
				}
				io->KeysDown[vkey] = true;
				break;
			}
			case WM_ACTIVATEAPP: {
				GlobalObjects::UpdateArcExports();
				if (!wParam) {
					io->KeysDown[GlobalObjects::ARC_GLOBAL_MOD1] = false;
					io->KeysDown[GlobalObjects::ARC_GLOBAL_MOD2] = false;
				}
				break;
			}
			// track current input language
			case WM_INPUTLANGCHANGE: {
				GlobalObjects::CURRENT_HKL = (HKL)lParam;
				break;
			}
			default:
				break;
		}
	} catch (const std::exception& e) {
		ARC_LOG_FILE("exception in mod_wnd");
		ARC_LOG_FILE(e.what());
		throw e;
	}

#if PERFORMANCE_LOG
	const auto& afterPoint = std::chrono::high_resolution_clock::now();
	const auto& diff = afterPoint - beforePoint;
	ARC_LOG_FILE(std::format("mod_wnd: {}", diff.count()).c_str());
#endif

	return uMsg;
}

namespace {
	uint64_t cbtcount = 0;
}
/* combat callback -- may be called asynchronously, use id param to keep track of order, first event id will be 2. return ignored */
/* at least one participant will be party/squad or minion of, or a buff applied by squad in the case of buff remove. not all statechanges present, see evtc statechange enum */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) {
	auto& eventWindow = EventWindow::instance();

	/* ev is null. dst will only be valid on tracking add. skillname will also be null */
	if (!ev) {

		/* notify tracking change */
		if (!src->elite) {

			/* add */
			if (src->prof) {
				eventWindow.AddData(std::format("==== cbtnotify ==== -> agent added: {}:{} ({}), instid: {}, prof: {}, elite: {}, self: {}, team: {}, subgroup: {}", src->name, dst->name, src->id, dst->id, static_cast<std::underlying_type_t<Prof>>(dst->prof), dst->elite, dst->self, src->team, dst->team));
				// p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
				// p += _snprintf_s(p, 400, _TRUNCATE, "agent added: %s:%s (%0llx), instid: %u, prof: %u, elite: %u, self: %u, team: %u, subgroup: %u\n", src->name, dst->name, src->id, dst->id, dst->prof, dst->elite, dst->self, src->team, dst->team);
			}

			/* remove */
			else {
				eventWindow.AddData(std::format("==== cbtnotify ==== -> agent removed: {} ({})", src->name, src->id));
				// p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
				// p += _snprintf_s(p, 400, _TRUNCATE, "agent removed: %s (%0llx)\n", src->name, src->id);
			}
		}

		/* target change */
		else if (src->elite == 1) {
			eventWindow.AddData(std::format("==== cbtnotify ==== -> new target: {}", src->id));
			// p += _snprintf_s(p, 400, _TRUNCATE, "==== cbtnotify ====\n");
			// p += _snprintf_s(p, 400, _TRUNCATE, "new target: %0llx\n", src->id);
		}
	}

	/* combat event. skillname may be null. non-null skillname will remain static until client exit. refer to evtc notes for complete detail */
	else {

		/* default names */
		if (!src->name || !strlen(src->name)) src->name = (char*)"(area)";
		if (!dst->name || !strlen(dst->name)) dst->name = (char*)"(area)";

		/* common */
		eventWindow.AddData(std::format("combatdemo: ==== cbtevent {} at {} ====", cbtcount, ev->time));
		// p += _snprintf_s(p, 400, _TRUNCATE, "combatdemo: ==== cbtevent %u at %llu ====\n", cbtcount, ev->time);
		eventWindow.AddData(std::format("source agent: {} ({}:{}, {}:{}), master: {}", src->name, ev->src_agent, ev->src_instid, static_cast<std::underlying_type_t<Prof>>(src->prof), src->elite, ev->src_master_instid));
		// p += _snprintf_s(p, 400, _TRUNCATE, "source agent: %s (%0llx:%u, %lx:%lx), master: %u\n", src->name, ev->src_agent, ev->src_instid, src->prof, src->elite, ev->src_master_instid);

		if (ev->dst_agent) {
			eventWindow.AddData(std::format("target agent: {} ({}:{}, {}:{})", dst->name, ev->dst_agent, ev->dst_instid, static_cast<std::underlying_type_t<Prof>>(dst->prof), dst->elite));
			// p += _snprintf_s(p, 400, _TRUNCATE, "target agent: %s (%0llx:%u, %lx:%lx)\n", dst->name, ev->dst_agent, ev->dst_instid, dst->prof, dst->elite);
		} else {
			eventWindow.AddData("target agent: n/a");
			// p += _snprintf_s(p, 400, _TRUNCATE, "target agent: n/a\n");
		}

		/* statechange */
		if (ev->is_statechange) {
			eventWindow.AddData(std::format("is_statechange: {}", static_cast<std::underlying_type_t<cbtstatechange>>(ev->is_statechange)));
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_statechange: %u\n", ev->is_statechange);
		}

		/* activation */
		else if (ev->is_activation) {
			eventWindow.AddData(std::format("is_activation: {}", ev->is_activation));
			eventWindow.AddData(std::format("skill: {}:{}", skillname, ev->skillid));
			eventWindow.AddData(std::format("ms_expected: {}", ev->value));
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_activation: %u\n", ev->is_activation);
			// p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			// p += _snprintf_s(p, 400, _TRUNCATE, "ms_expected: %d\n", ev->value);
		}

		/* buff remove */
		else if (ev->is_buffremove) {
			eventWindow.AddData(std::format("is_buffremove: {}", ev->is_buffremove));
			eventWindow.AddData(std::format("skill: {}:{}", skillname, ev->skillid));
			eventWindow.AddData(std::format("ms_expected: {}", ev->value));
			eventWindow.AddData(std::format("ms_intensity: {}", ev->buff_dmg));
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_buffremove: %u\n", ev->is_buffremove);
			// p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			// p += _snprintf_s(p, 400, _TRUNCATE, "ms_duration: %d\n", ev->value);
			// p += _snprintf_s(p, 400, _TRUNCATE, "ms_intensity: %d\n", ev->buff_dmg);
		}

		/* buff */
		else if (ev->buff) {

			/* damage */
			if (ev->buff_dmg) {
				eventWindow.AddData(std::format("is_buff: {}", ev->buff));
				eventWindow.AddData(std::format("skill: {}:{}", skillname, ev->skillid));
				eventWindow.AddData(std::format("dmg: {}", ev->buff_dmg));
				eventWindow.AddData(std::format("is_shields: {}", ev->is_shields));
				// p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
				// p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
				// p += _snprintf_s(p, 400, _TRUNCATE, "dmg: %d\n", ev->buff_dmg);
				// p += _snprintf_s(p, 400, _TRUNCATE, "is_shields: %u\n", ev->is_shields);
			}

			/* application */
			else {
				eventWindow.AddData(std::format("is_buff: {}", ev->buff));
				eventWindow.AddData(std::format("skill: {}:{}", skillname, ev->skillid));
				eventWindow.AddData(std::format("raw ms: {}", ev->value));
				eventWindow.AddData(std::format("overstack ms: {}", ev->overstack_value));
				// p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
				// p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
				// p += _snprintf_s(p, 400, _TRUNCATE, "raw ms: %d\n", ev->value);
				// p += _snprintf_s(p, 400, _TRUNCATE, "overstack ms: %u\n", ev->overstack_value);
			}
		}

		/* strike */
		else {
			eventWindow.AddData(std::format("is_buff: {}", ev->buff));
			eventWindow.AddData(std::format("skill: {}:{}", skillname, ev->skillid));
			eventWindow.AddData(std::format("dmg: {}", ev->value));
			eventWindow.AddData(std::format("is_moving: {}", ev->is_moving));
			eventWindow.AddData(std::format("is_ninety: {}", ev->is_ninety));
			eventWindow.AddData(std::format("is_flanking: {}", ev->is_flanking));
			eventWindow.AddData(std::format("is_shields: {}", ev->is_shields));
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_buff: %u\n", ev->buff);
			// p += _snprintf_s(p, 400, _TRUNCATE, "skill: %s:%u\n", skillname, ev->skillid);
			// p += _snprintf_s(p, 400, _TRUNCATE, "dmg: %d\n", ev->value);
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_moving: %u\n", ev->is_moving);
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_ninety: %u\n", ev->is_ninety);
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_flanking: %u\n", ev->is_flanking);
			// p += _snprintf_s(p, 400, _TRUNCATE, "is_shields: %u\n", ev->is_shields);
		}

		/* common */
		eventWindow.AddData(std::format("iff: {}", ev->iff));
		eventWindow.AddData(std::format("result: {}", ev->result));
		// p += _snprintf_s(p, 400, _TRUNCATE, "iff: %u\n", ev->iff);
		// p += _snprintf_s(p, 400, _TRUNCATE, "result: %u\n", ev->result);
		cbtcount += 1;
	}

	return 0;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init() {
	// windows init
	EventWindow::instance().Init();
#if _DEBUG
	DemoWindow::instance().Init();
	DemoTableWindow::instance().Init();
#endif

	arc_exports.imguivers = IMGUI_VERSION_NUM;
	arc_exports.out_name = "Demo Project";
	arc_exports.out_build = "0.1.0";

	/* for arcdps */
	arc_exports.sig = 0x00000011; // generate random number!
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.wnd_nofilter = mod_wnd;
	arc_exports.combat = mod_combat;
	arc_exports.imgui = mod_imgui;
	arc_exports.options_windows = mod_windows;
	// arc_exports.options_end = mod_options;
	// arc_exports.combat_local = mod_combat_local;

	return &arc_exports;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client load */
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, ImGuiContext* imguicontext, void* dxptr,
													 HMODULE new_arcdll, void* mallocfn,
													 void* freefn, UINT dxver) {
	ImGui::SetCurrentContext(imguicontext);
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

	ARC_DLL = new_arcdll;
	ARC_EXPORT_E6 = (arc_export_func_u64)GetProcAddress(ARC_DLL, "e6");
	ARC_EXPORT_E7 = (arc_export_func_u64)GetProcAddress(ARC_DLL, "e7");
	ARC_LOG_FILE = (e3_func_ptr)GetProcAddress(ARC_DLL, "e3");
	ARC_LOG = (e3_func_ptr)GetProcAddress(ARC_DLL, "e8");

	// dx11 not available in older arcdps versions
	if (dxver == 11) {
		auto swapChain = static_cast<IDXGISwapChain*>(dxptr);
		swapChain->GetDevice(__uuidof(d3d11Device), reinterpret_cast<void**>(&d3d11Device));
	} else {
		d3d9Device = static_cast<IDirect3DDevice9*>(dxptr);
	}

	// install imgui hooks
	PositioningComponentImGuiHook::InstallHooks(imguicontext);

	return mod_init;
}

/* release mod -- return ignored */
uintptr_t mod_release() {
	g_singletonManagerInstance.Shutdown();

	return 0;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client exit */
extern "C" __declspec(dllexport) void* get_release_addr() {
	return mod_release;
}

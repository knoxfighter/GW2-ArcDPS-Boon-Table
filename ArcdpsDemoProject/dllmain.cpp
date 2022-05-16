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

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init() {
	// windows init
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
	// arc_exports.combat = mod_combat;
	arc_exports.imgui = mod_imgui;
	// arc_exports.options_end = mod_options;
	arc_exports.options_windows = mod_windows;
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

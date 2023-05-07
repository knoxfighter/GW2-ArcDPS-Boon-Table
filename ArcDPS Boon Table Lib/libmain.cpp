#include "BoonWindowHandler.h"
#include "GlobalObjects.h"
#include "Logger.h"
#include "Settings.h"
#include "Tracker.h"

#include <ArcdpsExtension/arcdps_structs.h>
#include <ArcdpsExtension/IconLoader.h>
#include <ArcdpsExtension/KeyBindHandler.h>
#include <ArcdpsExtension/KeyInput.h>
#include <ArcdpsExtension/MumbleLink.h>
#include <ArcdpsExtension/UpdateChecker.h>
#include <ArcdpsExtension/Windows/PositioningComponent.h>
#include <ArcdpsExtension/Windows/Demo/DemoTableWindow.h>
#include <ArcdpsExtension/Windows/Demo/DemoWindow.h>

#include <ArcdpsUnofficialExtras//Definitions.h>

#include <imgui/imgui.h>

#include <d3d11.h>
#include <d3d9.h>
#include <format>
#include <Windows.h>

namespace {
	HMODULE ARC_DLL;
	IDirect3DDevice9* d3d9Device = nullptr;
	ID3D11Device* d3d11Device = nullptr;
	arcdps_exports arc_exports = {};

	bool initFailed = false;
}

uintptr_t mod_windows(const char* windowname) {
	if (!windowname) {
		// TODO
		// BoonWindowHandler::instance().DrawOptionCheckboxes();
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

	BoonWindowHandler::instance().Draw();
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

/* combat callback -- may be called asynchronously, use id param to keep track of order, first event id will be 2. return ignored */
/* at least one participant will be party/squad or minion of, or a buff applied by squad in the case of buff remove. not all statechanges present, see evtc statechange enum */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision) {
	// LOG_T("EventId: {}", id);
	Tracker::instance().Event(ev, src, dst, skillname, id, revision);

	return 0;
}

uintptr_t mod_options() {
	// TODO
	// SettingsUI::instance().Draw();

	return 0;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init() {
	bool loading_successful = true;
	std::string error_message = "Unknown error";

	UpdateChecker& updateChecker = UpdateChecker::instance();
	const auto& currentVersion = updateChecker.GetCurrentVersion(GlobalObjects::SELF_DLL);

	try {
		// init logger
		if (!GlobalObjects::IS_UNIT_TEST) {
			Log_::Init(false, "addons/logs/GW2-ArcDPS-Boon-Table/GW2-ArcDPS-Boon-Table.txt");

			Log_::SetLevel(spdlog::level::info);
		}

		// Setup iconLoader
		IconLoader::instance().Setup(GlobalObjects::SELF_DLL, d3d9Device, d3d11Device);

		// Clear old Files
		updateChecker.ClearFiles(GlobalObjects::SELF_DLL);

		// check for new version on github
		if (currentVersion) {
			GlobalObjects::UPDATE_STATE = std::move(
				updateChecker.CheckForUpdate(GlobalObjects::SELF_DLL, currentVersion.value(),
				                             "knoxfighter/GW2-ArcDPS-Boon-Table", false));
		}

		// generate tracker instance
		Tracker::instance();

		// TODO
		// Settings::instance().load();
		//
		// LoadAdditionalTranslations();
		// load current language
		// Localization::instance().ChangeLanguage(static_cast<gwlanguage>(Settings::instance().GetLanguage()));

		// windows init
		// TODO
		// BoonWindowHandler::instance().Init();
#if _DEBUG
		DemoWindow::instance().Init();
		DemoTableWindow::instance().Init();
#endif
	} catch (const std::exception& e) {
		loading_successful = false;
		error_message = "Error loading all icons: ";
		error_message.append(e.what());
	}

	arc_exports.imguivers = IMGUI_VERSION_NUM;
	arc_exports.out_name = "Boon Table";
	const std::string& version = currentVersion
									 ? updateChecker.GetVersionAsString(currentVersion.value())
									 : "Unknown";
	char* version_c_str = new char[version.length() + 1];
	strcpy_s(version_c_str, version.length() + 1, version.c_str());
	arc_exports.out_build = version_c_str;


	if (loading_successful) {
		/* for arcdps */
		arc_exports.sig = 0x64003268;
		arc_exports.size = sizeof(arcdps_exports);
		arc_exports.wnd_nofilter = mod_wnd;
		arc_exports.combat = mod_combat;
		arc_exports.imgui = mod_imgui;
		arc_exports.options_end = mod_options;
		arc_exports.options_windows = mod_windows;
		// arc_exports.combat_local = mod_combat_local;
	} else {
		initFailed = true;
		arc_exports.sig = 0;
		const std::string::size_type size = error_message.size();
		char* buffer = new char[size + 1]; //we need extra char for NUL
		memcpy(buffer, error_message.c_str(), size + 1);
		arc_exports.size = (uintptr_t)buffer;
	}

	return &arc_exports;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client load */
extern "C" __declspec(dllexport) ModInitSignature get_init_addr(const char* arcversion, ImGuiContext* imguictx, void* dxptr, HMODULE arcdll, MallocSignature mallocfn, FreeSignature freefn, UINT dxver) {
	ImGui::SetCurrentContext(imguictx);
	ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

	ARC_DLL = arcdll;
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
	PositioningComponentImGuiHook::InstallHooks(imguictx);

	return mod_init;
}

/* release mod -- return ignored */
uintptr_t mod_release() {
	// finish and clear updateState
	if (GlobalObjects::UPDATE_STATE) {
		GlobalObjects::UPDATE_STATE->FinishPendingTasks();
		GlobalObjects::UPDATE_STATE.reset(nullptr);
	}

	// TODO
	// Settings::instance().unload();

	g_singletonManagerInstance.Shutdown();

	Log_::FlushLogFile();

	if (!GlobalObjects::IS_UNIT_TEST)
	{
		Log_::Shutdown();
	}

	return 0;
}

/* export -- arcdps looks for this exported function and calls the address it returns on client exit */
extern "C" __declspec(dllexport) ModReleaseSignature get_release_addr() {
	return mod_release;
}

void language_changed_callback(Language pNewLanguage) {
	GlobalObjects::CURRENT_LANGUAGE = pNewLanguage;

	// TODO
	// if (Settings::instance().settings.language == LanguageSetting::LikeGame) {
	// 	Localization::SChangeLanguage(static_cast<gwlanguage>(GlobalObjects::CURRENT_LANGUAGE));
	// }
}

extern "C" __declspec(dllexport) void arcdps_unofficial_extras_subscriber_init(
	const ExtrasAddonInfo* pExtrasInfo, void* pSubscriberInfo) {
	// do not subscribe, if initialization called from arcdps failed.
	// MaxInfoVersion has to be higher to have enough space to hold this object
	if (!initFailed && pExtrasInfo->ApiVersion == 2 && pExtrasInfo->MaxInfoVersion >= 1) {
		ExtrasSubscriberInfoV1* subscriberInfo = static_cast<ExtrasSubscriberInfoV1*>(pSubscriberInfo);

		subscriberInfo->InfoVersion = 1;
		subscriberInfo->SubscriberName = "Boon Table";
		subscriberInfo->LanguageChangedCallback = language_changed_callback;
	}
}

/*
* arcdps combat api example
*/

#include <stdint.h>
#include <stdio.h>
#include <Windows.h>
#include <string>
#include <list>

#include "Player.h"
#include "Tracker.h"

/* arcdps export table */
typedef struct arcdps_exports {
	uintptr_t size; /* arcdps internal use, ignore */
	uintptr_t sig; /* pick a non-zero number that isn't used by other modules */
	char* out_name; /* name string */
	char* out_build; /* build string */
	void* wnd; /* wndproc callback, fn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) */
	void* combat; /* combat event callback, fn(cbtevent* ev, ag* src, ag* dst, char* skillname) */
	void* imgui; /* id3dd9::present callback, before imgui::render, fn() */
	void* options; /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
} arcdps_exports;

/* combat event */
typedef struct cbtevent {
	uint64_t time; /* timegettime() at time of event */
	uint64_t src_agent; /* unique identifier */
	uint64_t dst_agent; /* unique identifier */
	int32_t value; /* event-specific */
	int32_t buff_dmg; /* estimated buff damage. zero on application event */
	uint16_t overstack_value; /* estimated overwritten stack duration for buff application */
	uint16_t skillid; /* skill id */
	uint16_t src_instid; /* agent map instance id */
	uint16_t dst_instid; /* agent map instance id */
	uint16_t src_master_instid; /* master source agent map instance id if source is a minion/pet */
	uint8_t iss_offset; /* internal tracking. garbage */
	uint8_t iss_offset_target; /* internal tracking. garbage */
	uint8_t iss_bd_offset; /* internal tracking. garbage */
	uint8_t iss_bd_offset_target; /* internal tracking. garbage */
	uint8_t iss_alt_offset; /* internal tracking. garbage */
	uint8_t iss_alt_offset_target; /* internal tracking. garbage */
	uint8_t skar; /* internal tracking. garbage */
	uint8_t skar_alt; /* internal tracking. garbage */
	uint8_t skar_use_alt; /* internal tracking. garbage */
	uint8_t iff; /* from iff enum */
	uint8_t buff; /* buff application, removal, or damage event */
	uint8_t result; /* from cbtresult enum */
	uint8_t is_activation; /* from cbtactivation enum */
	uint8_t is_buffremove; /* buff removed. src=relevant, dst=caused it (for strips/cleanses). from cbtr enum  */
	uint8_t is_ninety; /* source agent health was over 90% */
	uint8_t is_fifty; /* target agent health was under 50% */
	uint8_t is_moving; /* source agent was moving */
	uint8_t is_statechange; /* from cbtstatechange enum */
	uint8_t is_flanking; /* target agent was not facing source */
	uint8_t is_shields; /* all or partial damage was vs barrier/shield */
	uint8_t pad63; /* internal tracking. garbage */
	uint8_t pad64; /* internal tracking. garbage */
} cbtevent;

/* agent short */
typedef struct ag {
	char* name; /* agent name. may be null. valid only at time of event. utf8 */
	uintptr_t id; /* agent unique identifier */
	uint32_t prof; /* profession at time of event. refer to evtc notes for identification */
	uint32_t elite; /* elite spec at time of event. refer to evtc notes for identification */
	uint32_t self; /* 1 if self, 0 if not */
} ag;

/* proto/globals */
uint32_t cbtcount = 0;
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

Tracker tracker;

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
	//ImGui::SetCurrentContext((ImGuiContext*)imguicontext);
	return mod_init;
}

/* export -- arcdps looks for this exported function and calls the address it returns */
extern "C" __declspec(dllexport) void* get_release_addr() {
	arcvers = 0;
	return mod_release;
}

/* initialize mod -- return table that arcdps will use for callbacks */
arcdps_exports* mod_init() {
	/* demo */
	AllocConsole();

	/* big buffer */
	char buff[4096];
	char* p = &buff[0];
	p += _snprintf(p, 400, "==== mod_init ====\n");
	p += _snprintf(p, 400, "arcdps: %s\n", arcvers);

	/* print */
	DWORD written = 0;
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(hnd, &buff[0], p - &buff[0], &written, 0);

	/* for arcdps */
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.out_name = "Boon Table";
	arc_exports.out_build = "0.1";
	arc_exports.sig = 0x64003268;//from random.org
	arc_exports.wnd = mod_wnd;
	arc_exports.combat = mod_combat;
	return &arc_exports;
}

/* release mod -- return ignored */
uintptr_t mod_release() {
	FreeConsole();
	return 0;
}

/* window callback -- return is assigned to umsg (return zero to not be processed by arcdps or game) */
uintptr_t mod_wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	/* big buffer */
	char buff[4096];
	char* p = &buff[0];

	/* common */
	p += _snprintf(p, 400, "==== wndproc %llx ====\n", hWnd);
	p += _snprintf(p, 400, "umsg %u, wparam %lld, lparam %lld\n", uMsg, wParam, lParam);

	/* print */
	DWORD written = 0;
	HANDLE hnd = GetStdHandle(STD_OUTPUT_HANDLE);
	//WriteConsoleA(hnd, &buff[0], p - &buff[0], &written, 0);
	return uMsg;
}

/* combat callback -- may be called asynchronously. return ignored */
/* one participant will be party/squad, or minion of. no spawn statechange events. despawn statechange only on marked boss npcs */
uintptr_t mod_combat(cbtevent* ev, ag* src, ag* dst, char* skillname) {
	/* big buffer */
	char buff[4096];
	char* p = &buff[0];
	wchar_t buffw[4096];

	/* ev is null. dst will only be valid on tracking add. skillname will also be null */
	if (!ev) {

		/* notify tracking change */
		if (!src->elite) {

			/* add */
			if (src->prof) {
				p += _snprintf(p, 400, "==== cbtnotify ====\n");
				// self flag disabled - always 1
				p += _snprintf(p, 400, "agent added: %s:%s (%0llx), prof: %u, elite: %u, self: %u\n", src->name, dst->name, src->id, dst->prof, dst->elite, dst->self);
				tracker.addPlayer(src->id, std::string(src->name));
			}

			/* remove */
			else {
				p += _snprintf(p, 400, "==== cbtnotify ====\n");
				p += _snprintf(p, 400, "agent removed: %s (%llx)\n", src->name, src->id);
				tracker.removePlayer(src->id);
			}
		}

		/* notify target change */
		else if (src->elite == 1) {
			
		}
	}

	/* combat event. skillname may be null. non-null skillname will remain static until module is unloaded. refer to evtc notes for complete detail */
	else {

		/* default names */
		if (!src->name || !strlen(src->name)) src->name = "(area)";
		if (!dst->name || !strlen(dst->name)) dst->name = "(area)";

		/* common */
		p += _snprintf(p, 400, "==== cbtevent %u at %llu ====\n", cbtcount, ev->time);
		p += _snprintf(p, 400, "source agent: %s (%llx:%u, %lx:%lx), master: %u\n", src->name, ev->src_agent, ev->src_instid, src->prof, src->elite, ev->src_master_instid);
		if (ev->dst_agent) p += _snprintf(p, 400, "target agent: %s (%llx:%u, %lx:%lx)\n", dst->name, ev->dst_agent, ev->dst_instid, dst->prof, dst->elite);
		else p += _snprintf(p, 400, "target agent: n/a\n");

		/* statechange */
		if (ev->is_statechange) {
			p += _snprintf(p, 400, "is_statechange: %u\n", ev->is_statechange);
		}

		/* activation */
		else if (ev->is_activation) {
			p += _snprintf(p, 400, "is_activation: %u\n", ev->is_activation);
			p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf(p, 400, "ms_expected: %d\n", ev->value);
		}

		/* buff remove */
		else if (ev->is_buffremove) {
			p += _snprintf(p, 400, "is_buffremove: %u\n", ev->is_buffremove);
			p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf(p, 400, "ms_duration: %d\n", ev->value);
			p += _snprintf(p, 400, "ms_intensity: %d\n", ev->buff_dmg);
		}

		/* buff */
		else if (ev->buff) {

			/* damage */
			if (ev->buff_dmg) {
				p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
				p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
				p += _snprintf(p, 400, "dmg: %d\n", ev->buff_dmg);
				p += _snprintf(p, 400, "is_shields: %u\n", ev->is_shields);
			}

			/* application */
			else {
				p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
				p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
				p += _snprintf(p, 400, "raw ms: %d\n", ev->value);
				p += _snprintf(p, 400, "overstack ms: %u\n", ev->overstack_value);
			}
		}

		/* physical */
		else {
			p += _snprintf(p, 400, "is_buff: %u\n", ev->buff);
			p += _snprintf(p, 400, "skill: %s:%u\n", skillname, ev->skillid);
			p += _snprintf(p, 400, "dmg: %d\n", ev->value);
			p += _snprintf(p, 400, "is_moving: %u\n", ev->is_moving);
			p += _snprintf(p, 400, "is_ninety: %u\n", ev->is_ninety);
			p += _snprintf(p, 400, "is_flanking: %u\n", ev->is_flanking);
			p += _snprintf(p, 400, "is_shields: %u\n", ev->is_shields);
		}

		/* common */
		p += _snprintf(p, 400, "iff: %u\n", ev->iff);
		p += _snprintf(p, 400, "result: %u\n", ev->result);
		cbtcount += 1;
	}

	/* print */
	DWORD written = 0;
	int32_t rc = MultiByteToWideChar(CP_UTF8, 0, &buff[0], MAX_PATH, &buffw[0], 4096);
	buffw[rc] = 0;
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), &buffw[0], wcslen(&buffw[0]), &written, 0);
	return 0;
}
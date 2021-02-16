#pragma once
#include <string>

//enums and structs:
/* is friend/foe */
enum iff {
	IFF_FRIEND,
	IFF_FOE,
	IFF_UNKNOWN // or uncertain
};

/* combat result (physical) */
enum cbtresult {
	CBTR_NORMAL, // good physical hit
	CBTR_CRIT, // physical hit was crit
	CBTR_GLANCE, // physical hit was glance
	CBTR_BLOCK, // physical hit was blocked eg. mesmer shield 4
	CBTR_EVADE, // physical hit was evaded, eg. dodge or mesmer sword 2
	CBTR_INTERRUPT, // physical hit interrupted something
	CBTR_ABSORB, // physical hit was "invlun" or absorbed eg. guardian elite
	CBTR_BLIND, // physical hit missed
	CBTR_KILLINGBLOW, // hit was killing hit
	CBTR_DOWNED, // hit was downing hit
};

/* combat activation */
enum cbtactivation {
	ACTV_NONE, // not used - not this kind of event
	ACTV_NORMAL, // activation without quickness
	ACTV_QUICKNESS, // activation with quickness
	ACTV_CANCEL_FIRE, // cancel with reaching channel time
	ACTV_CANCEL_CANCEL, // cancel without reaching channel time
	ACTV_RESET // animation completed fully
};

/* combat state change */
enum cbtstatechange {
	CBTS_NONE, // not used - not this kind of event
	CBTS_ENTERCOMBAT, // src_agent entered combat, dst_agent is subgroup
	CBTS_EXITCOMBAT, // src_agent left combat
	CBTS_CHANGEUP, // src_agent is now alive
	CBTS_CHANGEDEAD, // src_agent is now dead
	CBTS_CHANGEDOWN, // src_agent is now downed
	CBTS_SPAWN, // src_agent is now in game tracking range
	CBTS_DESPAWN, // src_agent is no longer being tracked
	CBTS_HEALTHUPDATE, // src_agent has reached a health marker. dst_agent = percent * 10000 (eg. 99.5% will be 9950)
	CBTS_LOGSTART, // log start. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id)
	CBTS_LOGEND, // log end. value = server unix timestamp **uint32**. buff_dmg = local unix timestamp. src_agent = 0x637261 (arcdps id)
	CBTS_WEAPSWAP, // src_agent swapped weapon set. dst_agent = current set id (0/1 water, 4/5 land)
	CBTS_MAXHEALTHUPDATE, // src_agent has had it's maximum health changed. dst_agent = new max health
	CBTS_POINTOFVIEW, // src_agent will be agent of "recording" player
	CBTS_LANGUAGE, // src_agent will be text language
	CBTS_GWBUILD, // src_agent will be game build
	CBTS_SHARDID, // src_agent will be sever shard id
	CBTS_REWARD, // src_agent is self, dst_agent is reward id, value is reward type. these are the wiggly boxes that you get
	CBTS_BUFFINITIAL, // combat event that will appear once per buff per agent on logging start (zero duration, buff==18)
	CBTS_POSITION, // src_agent changed, cast float* p = (float*)&dst_agent, access as x/y/z (float[3])
	CBTS_VELOCITY, // src_agent changed, cast float* v = (float*)&dst_agent, access as x/y/z (float[3])
	CBTS_FACING, // src_agent changed, cast float* f = (float*)&dst_agent, access as x/y (float[2])
	CBTS_TEAMCHANGE, // src_agent change, dst_agent new team id
	CBTS_ATTACKTARGET, // src_agent is an attacktarget, dst_agent is the parent agent (gadget type), value is the current targetable state
	CBTS_TARGETABLE // dst_agent is new target-able state (0 = no, 1 = yes. default yes)
};

/* combat buff remove type */
enum cbtbuffremove {
	CBTB_NONE, // not used - not this kind of event
	CBTB_ALL, // last/all stacks removed (sent by server)
	CBTB_SINGLE, // single stack removed (sent by server). will happen for each stack on cleanse
	CBTB_MANUAL, // single stack removed (auto by arc on ooc or all stack, ignore for strip/cleanse calc, use for in/out volume)
};

/* custom skill ids */
enum cbtcustomskill {
	CSK_RESURRECT = 1066, // not custom but important and unnamed
	CSK_BANDAGE = 1175, // personal healing only
	CSK_DODGE = 65001 // will occur in is_activation==normal event
};

/* language */
enum gwlanguage {
	GWL_ENG = 0,
	GWL_FRE = 2,
	GWL_GEM = 3,
	GWL_SPA = 4,
};

enum prof : uint32_t {
	PROF_UNKNOWN = 0,
	PROF_GUARD = 1,
	PROF_WARRIOR = 2,
	PROF_ENGINEER = 3,
	PROF_RANGER = 4,
	PROF_THIEF = 5,
	PROF_ELE  = 6,
	PROF_MESMER = 7,
	PROF_NECRO = 8,
	PROF_RENEGADE = 9,
};

/* arcdps export table */
typedef struct arcdps_exports {
	uintptr_t size; /* size of exports table */
	uint32_t sig; /* pick a number between 0 and uint32_t max that isn't used by other modules */
	uint32_t imguivers; /* set this to IMGUI_VERSION_NUM. if you don't use imgui, 18000 (as of 2021-02-02) */
	char* out_name; /* name string */
	char* out_build; /* build string */
	void* wnd_nofilter; /* wndproc callback, fn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) */
	void* combat; /* combat event callback, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
	void* imgui; /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
	void* options_end; /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
	void* combat_local;  /* combat event callback like area but from chat log, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
	void* wnd_filter; /* wndproc callback like above, input filered using modifiers */
	void* options_windows; /* called once per 'window' option checkbox, with null at the end, non-zero return disables drawing that option, fn(char* windowname) */
} arcdps_exports;

/* combat event logging (revision 1, when header[12] == 1) */
typedef struct cbtevent {
	uint64_t time;
	uint64_t src_agent;
	uint64_t dst_agent;
	int32_t value;
	int32_t buff_dmg;
	uint32_t overstack_value;
	uint32_t skillid;
	uint16_t src_instid;
	uint16_t dst_instid;
	uint16_t src_master_instid;
	uint16_t dst_master_instid;
	uint8_t iff;
	uint8_t buff;
	uint8_t result;
	uint8_t is_activation;
	uint8_t is_buffremove;
	uint8_t is_ninety;
	uint8_t is_fifty;
	uint8_t is_moving;
	uint8_t is_statechange;
	uint8_t is_flanking;
	uint8_t is_shields;
	uint8_t is_offcycle;
	uint8_t pad61;
	uint8_t pad62;
	uint8_t pad63;
	uint8_t pad64;
} cbtevent;

/* agent short */
typedef struct ag {
	char* name; /* agent name. may be null. valid only at time of event. utf8 */
	uintptr_t id; /* agent unique identifier */
	prof prof; /* profession at time of event. refer to evtc notes for identification */
	uint32_t elite; /* elite spec at time of event. refer to evtc notes for identification */
	uint32_t self; /* 1 if self, 0 if not */
	uint16_t team; /* sep21+ */
} ag;

bool is_player(ag* new_player);

// additional enum for alignment
enum class Alignment {
	Unaligned,
	Left,
	Center,
	Right
};

std::string to_string(Alignment alignment);

// additional enum for progressbar color
enum class ProgressBarColoringMode {
	Uncolored,
	ByProfession,
	ByPercentage
};

std::string to_string(ProgressBarColoringMode coloringMode);

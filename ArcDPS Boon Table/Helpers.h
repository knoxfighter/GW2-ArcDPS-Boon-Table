#pragma once

#include <cstdint>
#include <optional>

#include "extension/arcdps_structs.h"
#include "imgui/imgui.h"
#include "modernIni/modernIni/modernIniMacros.h"

extern uint64_t current_time;

#define PERCENT_STR(x, dura) {							\
		u32 per = (u32)(x / (f32)(dura) * 100.0f);		\
		per = per > 100 ? 100 : per;					\
		if (per>99) ImGui::SameLine(3);					\
		ImGui::Text("%u%%", per);						\
		}

#define __VERSION__ (__DATE__)

uint64_t getCurrentTime();

int32_t getBuffApplyDuration(cbtevent* ev);

inline float discretize(float a);

bool floatCmp(float a, float b);

typedef void (*arc_color_func)(ImVec4**);
extern arc_color_func arc_export_e5;

typedef void (*arc_log_func_ptr)(const char* str);
extern arc_log_func_ptr arc_log_file;
extern arc_log_func_ptr arc_log;

// additional enum for progressbar color
enum class ProgressBarColoringMode {
	Uncolored,
	ByProfession,
	ByPercentage,

	LAST_ENTRY
};

std::string to_string(ProgressBarColoringMode coloringMode);

std::string to_string(const ImVec4& vec4);
std::optional<ImVec4> ImVec4_color_from_string(const std::string& vec4str);

extern HMODULE self_dll;
extern LPVOID mapViewOfMumbleFile;
extern bool isWvW;

// ImGui function to display question marks with tooltips
static void HelpMarker(const char* desc)
{
	ImGui::PushStyleColor(ImGuiCol_Text, {1.f, 1.f, 1.f, .5f});
    ImGui::TextUnformatted("(?)");
	ImGui::PopStyleColor();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

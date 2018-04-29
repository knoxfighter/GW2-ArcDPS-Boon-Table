#pragma once
#include <chrono>

#define PERCENT_STR(x, dura) {							\
		u32 per = (u32)(x / (f32)(dura) * 100.0f);		\
		per = per > 100 ? 100 : per;					\
		if (per>99) ImGui::SameLine(3);					\
		ImGui::Text("%u%%", per);						\
		}

uint64_t getCurrentTime();
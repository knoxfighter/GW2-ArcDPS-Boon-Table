#include "Helpers.h"

#include <sstream>

#include "Lang.h"

uint64_t getCurrentTime()
{
	return current_time;
}

int32_t getBuffApplyDuration(cbtevent * ev)
{
	if (!ev) return 0;

	if (!ev->is_offcycle)//is normal buff apply
	{
		return ev->value - ev->overstack_value;
	}
	else//is buff extension
	{
		return ev->value;
	}
}

uint64_t current_time = 0;

inline float discretize(float a)
{
	return floorf(a * 100.0f) / 100.0f;
}

bool floatCmp(float a, float b)
{
	float aa = discretize(a);
	float bb = discretize(b);
	if (aa == bb)
		return false;
	return aa < bb;
}

std::string to_string(ProgressBarColoringMode coloringMode) {
	switch (coloringMode) {
	case ProgressBarColoringMode::Uncolored:
		return lang.translate(LangKey::ColoringModeDefault);
	case ProgressBarColoringMode::ByProfession:
		return lang.translate(LangKey::ColoringModeByProfession);
	case ProgressBarColoringMode::ByPercentage:
		return lang.translate(LangKey::ColoringModeByPercentage);
	}
}

std::string to_string(const ImVec4& vec4) {
	std::string colorCombined;
	colorCombined.append(std::to_string(vec4.x));
	colorCombined.append("|");
	colorCombined.append(std::to_string(vec4.y));
	colorCombined.append("|");
	colorCombined.append(std::to_string(vec4.z));
	colorCombined.append("|");
	colorCombined.append(std::to_string(vec4.w));
	return colorCombined;
}

std::optional<ImVec4> ImVec4_color_from_string(const std::string& vec4str) {
	// check if string is suitable
	size_t count = std::count(vec4str.begin(), vec4str.end(), '|');
	if (count < 3) {
		// read color invalid
		return std::nullopt;
	}
	
	std::stringstream stream(vec4str);
	std::string buf;
	float val[4]{};
	// Iterate over 4 first sections split by '|'
	for (int i = 0; std::getline(stream, buf, '|') && i < 4; ++i) {
		val[i] = std::stof(buf);
	}
	return ImVec4(val[0], val[1], val[2], val[3]);
}

bool isWvW = false;

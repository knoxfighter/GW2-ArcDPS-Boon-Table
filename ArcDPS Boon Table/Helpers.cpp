#include "Helpers.h"

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

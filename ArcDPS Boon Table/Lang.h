#pragma once

#include <string>

enum class LangKey {
	Left,
	Right,
	Center,
	Unaligned
};

class Lang {
public:
	std::string translate(LangKey key);

	Lang() = default;

	// copy/move delete
	Lang(const Lang& other) = delete;
	Lang(Lang&& other) noexcept = delete;
	Lang& operator=(const Lang& other) = delete;
	Lang& operator=(Lang&& other) noexcept = delete;
};

extern Lang lang;

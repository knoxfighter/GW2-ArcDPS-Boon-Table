#pragma once

#include <ArcdpsExtension/arcdps_structs_slim.h>

#include <spdlog/spdlog.h>

#include <magic_enum.hpp>
#include <memory>
#include <ranges>
#include <source_location>

namespace Log_ {
	void Init(bool pRotateOnOpen, const char* pLogPath);
	void Shutdown();
	void SetLevel(spdlog::level::level_enum pLevel);
	void LockLogger();
	void FlushLogFile();
	
	constexpr std::string GetFileName(const std::string& path) {
	    return path.substr(path.find_last_of("/\\") + 1);
	}

	inline std::shared_ptr<spdlog::logger> LOGGER;
}

template <typename E>
requires std::is_enum_v<E>
struct std::formatter<E> : std::formatter<std::string_view> {
  auto format(E c, auto& ctx) { return formatter<std::string_view>::format(magic_enum::enum_name(c), ctx); }
};

#ifdef _DEBUG
/**
 * Trace logging to file
 */
#define LOG_T(pFormatString, ...) Log_::LOGGER->trace("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
/**
 * Debug logging to file
 */
#define LOG_D(pFormatString, ...) Log_::LOGGER->debug("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
/**
 * Info logging to file
 */
#define LOG_I(pFormatString, ...) Log_::LOGGER->info("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
/**
 * Warning logging to file
 */
#define LOG_W(pFormatString, ...) Log_::LOGGER->warn("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
/**
 * Error logging to file
 */
#define LOG_E(pFormatString, ...) Log_::LOGGER->error("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
/**
 * Critical logging to file
 */
#define LOG_C(pFormatString, ...) Log_::LOGGER->critical("{}|{}|" pFormatString, Log_::GetFileName(std::source_location::current().file_name()), std::source_location::current().function_name(), ##__VA_ARGS__)
#else
#define LOG_T(pFormatString, ...)
#define LOG_D(pFormatString, ...) 
#define LOG_I(pFormatString, ...)
#define LOG_W(pFormatString, ...)
#define LOG_E(pFormatString, ...)
#define LOG_C(pFormatString, ...)
#endif

#include "Logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>

void Log_::Init(bool pRotateOnOpen, const char* pLogPath) {
	if (Log_::LOGGER != nullptr)
	{
		LOG_W("Skipping logger initialization since logger is not nullptr");
		return;
	}

	LOGGER = spdlog::rotating_logger_mt<spdlog::async_factory>("arcdps_healing_stats", pLogPath, 256*1024*1024, 8, pRotateOnOpen);
	LOGGER->set_pattern("%b %d %H:%M:%S.%f %t %L %v");
	spdlog::flush_every(std::chrono::seconds(5));
}

void Log_::Shutdown() {
	LOGGER = nullptr;
	spdlog::shutdown();
}

static std::atomic_bool LoggerLocked = false;
void Log_::SetLevel(spdlog::level::level_enum pLevel) {
	if (pLevel < 0 || pLevel >= spdlog::level::n_levels)
	{
		Log_::LOGGER->warn("{}|{}|" "Not setting level to {} since level is out of range", std::source_location::current().file_name(), std::source_location::current().function_name(),pLevel);
		return;
	}
	if (LoggerLocked == true)
	{
		LOG_W("Not setting level to {} since logger is locked", pLevel);
		return;
	}

	LOGGER->set_level(pLevel);
	LOG_I("Changed level to {}", pLevel);
}

void Log_::LockLogger() {
	LoggerLocked = true;
	LOG_I("Locked logger");
}

void Log_::FlushLogFile() {
	LOGGER->flush();
}

#include "milo/logging/Log.h"
#include <spdlog/async_logger.h>
#include "spdlog/sinks/stdout_color_sinks.h"

namespace milo {

	static inline SharedPtr<spdlog::logger> createLogger() noexcept {
		SharedPtr<spdlog::logger> logger = spdlog::stdout_color_mt("Milo");
		logger->set_pattern("%^[%D %H:%M:%S][%n][%l]: %v%$");
		return logger;
	}

	SharedPtr<spdlog::logger> Log::s_Logger = createLogger();
	Log::Level Log::s_Level = Log::Level::Info;

	Log::Level Log::level() {
		return s_Level;
	}

	void Log::level(Log::Level level) {
		s_Level = level;
		milo::Log::s_Logger->set_level(static_cast<spdlog::level::level_enum>(level));
	}

	void Log::trace(const String& message) {
		milo::Log::s_Logger->trace(message);
	}

	void Log::info(const String& message) {
		milo::Log::s_Logger->info(message);
	}

	void Log::debug(const String& message) {
		milo::Log::s_Logger->debug(message);
	}

	void Log::warn(const String& message) {
		milo::Log::s_Logger->warn(message);
	}

	void Log::error(const String& message) {
		milo::Log::s_Logger->error(message);
	}

	void Log::init() {
		s_Logger = spdlog::stdout_color_mt("Milo");
		s_Logger->set_pattern("[%D %H:%M:%S][%n]%^[%l]: %v%$");
	}

}
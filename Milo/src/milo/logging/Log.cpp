#include "milo/logging/Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace milo {

	std::shared_ptr<spdlog::logger> Log::s_Logger = nullptr;
#ifdef _DEBUG
	Log::Level Log::s_Level = Log::Level::Debug;
#else
	Log::Level Log::s_Level = Log::Level::Info;
#endif

	Log::Level Log::level() {
		return s_Level;
	}

	void Log::level(Log::Level level) {
		s_Level = level;
		milo::Log::s_Logger->set_level(static_cast<spdlog::level::level_enum>(level));
	}

	void Log::info(const String& message) {
		milo::Log::s_Logger->info(message);
	}

#ifdef _DEBUG
	void Log::debug(const String& message) {
		milo::Log::s_Logger->debug(message);
	}
#endif

	void Log::warn(const String& message) {
		milo::Log::s_Logger->warn(message);
	}

	void Log::error(const String& message) {
		milo::Log::s_Logger->error(message);
	}

	void Log::init() {
		s_Logger = spdlog::stdout_color_mt("Milo");
		s_Logger->set_pattern("%^[%D %H:%M:%S][%n][%l]: %v%$");
#ifdef _DEBUG
		s_Logger->set_level(spdlog::level::debug);
#endif
	}
}
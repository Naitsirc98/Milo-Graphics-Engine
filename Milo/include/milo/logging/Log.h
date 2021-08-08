#pragma once

#include "milo/common/Common.h"
#include <spdlog/spdlog.h>

#define LOG_TRACE(message) milo::Log::trace(UTAD_DETAILED_MESSAGE((message))
#define LOG_INFO(message) milo::Log::info(UTAD_DETAILED_MESSAGE((message))
#define LOG_DEBUG(message) milo::Log::debug(UTAD_DETAILED_MESSAGE((message))
#define LOG_WARN(message) milo::Log::warn(UTAD_DETAILED_MESSAGE((message))
#define LOG_ERROR(message) milo::Log::error(UTAD_DETAILED_MESSAGE((message))

namespace milo {

	class Log {

	public:
		enum class Level {
			Trace = spdlog::level::trace,
			Info = spdlog::level::info,
			Debug = spdlog::level::debug,
			Warning = spdlog::level::warn,
			Error = spdlog::level::err
		};

	public:
		static Log::Level level();
		static void level(Log::Level level);

		static void trace(const String& message);
		template<typename... Args>
		static void trace(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->trace(fmt, std::forward<Args>(args)...);
		}

		static void info(const String& message);
		template<typename... Args>
		static void info(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->info(fmt, std::forward<Args>(args)...);
		}

		static void debug(const String& message);
		template<typename... Args>
		static void debug(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->debug(fmt, std::forward<Args>(args)...);
		}

		static void warn(const String& message);
		template<typename... Args>
		static void warn(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->warn(fmt, std::forward<Args>(args)...);
		}

		static void error(const String& message);
		template<typename... Args>
		static void error(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->error(fmt, std::forward<Args>(args)...);
		}

		static void init();

	private:
		static SharedPtr<spdlog::logger> s_Logger;
		static Level s_Level;
	};

}
#pragma once

#include "milo/common/Common.h"
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#define LOG_DEBUG(message) milo::Log::debug(MILO_DETAILED_MESSAGE((message))
#else
#define LOG_DEBUG(message)
#endif
#define LOG_INFO(message) milo::Log::info(MILO_DETAILED_MESSAGE((message))
#define LOG_WARN(message) milo::Log::warn(MILO_DETAILED_MESSAGE((message))
#define LOG_ERROR(message) milo::Log::error(MILO_DETAILED_MESSAGE((message))

namespace milo {

	class Log {
		friend class MiloSubSystemManager;

	public:
		enum class Level {
#ifdef _DEBUG
			Debug = spdlog::level::debug,
#endif
			Info = spdlog::level::info,
			Warning = spdlog::level::warn,
			Error = spdlog::level::err
		};

	public:
		static Log::Level level();
		static void level(Log::Level level);

#ifdef _DEBUG
		static void debug(const String& message);
		template<typename... Args>
		static void debug(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->debug(fmt, std::forward<Args>(args)...);
		}
#else
		inline static void debug(const String& message) {}
		template<typename... Args>
		inline static void debug(fmt::format_string<Args...> fmt, Args &&...args) {}
#endif

		static void info(const String& message);
		template<typename... Args>
		static void info(fmt::format_string<Args...> fmt, Args &&...args)
		{
			s_Logger->info(fmt, std::forward<Args>(args)...);
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

	private:
		static void init();
		static void shutdown();

	private:
		static Shared<spdlog::logger> s_Logger;
		static Level s_Level;
	};

}
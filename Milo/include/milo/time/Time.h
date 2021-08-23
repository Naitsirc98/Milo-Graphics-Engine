#pragma once

#include "milo/common/Strings.h"
#include <chrono>
#include <iomanip>

namespace milo {

	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

	struct LocalDateTime {
		const tm time;

		inline String format(const String& pattern) {
			StringStream ss;
			ss << std::put_time(&time, pattern.c_str());
			return ss.str();
		}

		inline static LocalDateTime now() {
			auto now = std::chrono::system_clock::now();
			auto in_time_t = std::chrono::system_clock::to_time_t(now);
			return LocalDateTime(std::localtime(&in_time_t));
		}

		inline static LocalDateTime of(const tm* tm) {
			return LocalDateTime(tm);
		}

		inline bool operator==(const LocalDateTime& other) noexcept {
			return memcmp(&time, &other.time, sizeof(tm)) == 0;
		}

		inline bool operator!=(const LocalDateTime& other) noexcept {
			return !this->operator==(other);
		}

	private:
		explicit LocalDateTime(const tm* time) : time(*time) {}
	};

	template<>
	inline String str(const LocalDateTime& value) {
		StringStream ss;
		ss << std::put_time(&value.time, "%Y-%m-%d %X");
		return ss.str();
	}

	class Time {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static float s_RawDeltaTime;
		static float s_DeltaTime;
		static size_t s_Frame;
		static TimePoint s_StartTimeSeconds;
		static size_t s_Ups;
		static size_t s_Fps;
	public:
		static float now() noexcept;
		static float seconds() noexcept;
		static float millis() noexcept;
		static float nanos() noexcept;
		static float rawDeltaTime() noexcept;
		static float deltaTime() noexcept;
		static size_t frame() noexcept;
		static size_t ups() noexcept;
		static size_t fps() noexcept;
	private:
		static void init();
		static void shutdown();
	public:
		Time() = delete;
	};

	struct Stopwatch {
		const float startTime;
		explicit Stopwatch(float startTime) noexcept : startTime(startTime) {}
		 inline float elapsed() const {return Time::now() - startTime;}
	};
}



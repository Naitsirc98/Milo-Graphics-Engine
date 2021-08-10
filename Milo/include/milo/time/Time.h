#pragma once

#include <chrono>

namespace milo {

	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

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
		static float rawDeltaTime() noexcept;
		static float deltaTime() noexcept;
		static size_t frame() noexcept;
		static size_t ups() noexcept;
		static size_t fps() noexcept;
	private:
		static void init();
	public:
		Time() = delete;
	};

	struct Stopwatch {
		const float startTime;
		explicit Stopwatch(float startTime) noexcept : startTime(startTime) {}
		[[nodiscard]] inline float elapsed() const {return Time::now() - startTime;}
	};
}
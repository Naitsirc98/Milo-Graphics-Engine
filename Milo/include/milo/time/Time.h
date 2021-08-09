#pragma once

#include <chrono>

namespace milo {

	class Time {
		friend class MiloEngine;
	private:
		static float s_RawDeltaTime;
		static float s_DeltaTime;
		static size_t s_Frame;
	public:
		static float now() noexcept;
		static float rawDeltaTime() noexcept;
		static float deltaTime() noexcept;
		static size_t frame() noexcept;
	public:
		Time() = delete;
	};

	struct Stopwatch {
		const float startTime;
		explicit Stopwatch(float startTime) noexcept : startTime(startTime) {}
		[[nodiscard]] inline float elapsed() const {return Time::now() - startTime;}
	};
}
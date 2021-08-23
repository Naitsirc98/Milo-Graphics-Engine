#include "milo/time/Time.h"
#include <GLFW/glfw3.h>

#define NANOS_TO_SECONDS 1e-9f
#define NANOS_TO_MILLIS 1e-6f

namespace milo {

	using namespace std::chrono;

	float Time::s_RawDeltaTime = 0.0f;
	float Time::s_DeltaTime = 0.0f;
	size_t Time::s_Frame = 0;
	TimePoint Time::s_StartTimeSeconds;
	size_t Time::s_Ups = 0;
	size_t Time::s_Fps = 0;

	float Time::now() noexcept {
		const TimePoint now = std::chrono::high_resolution_clock::now();
		return static_cast<float>(duration_cast<nanoseconds>(now - s_StartTimeSeconds).count() * NANOS_TO_SECONDS);
	}

	float Time::seconds() noexcept {
		return now();
	}

	float Time::millis() noexcept {
		const TimePoint now = std::chrono::high_resolution_clock::now();
		return static_cast<float>(duration_cast<nanoseconds>(now - s_StartTimeSeconds).count() * NANOS_TO_MILLIS);
	}

	float Time::nanos() noexcept {
		const TimePoint now = std::chrono::high_resolution_clock::now();
		return static_cast<float>(duration_cast<nanoseconds>(now - s_StartTimeSeconds).count());
	}

	float Time::rawDeltaTime() noexcept {
		return s_RawDeltaTime;
	}

	float Time::deltaTime() noexcept {
		return s_DeltaTime;
	}

	size_t Time::frame() noexcept {
		return s_Frame;
	}

	size_t Time::ups() noexcept {
		return s_Ups;
	}

	size_t Time::fps() noexcept {
		return s_Fps;
	}

	void Time::init() {
		s_StartTimeSeconds = std::chrono::high_resolution_clock::now();
	}

	void Time::shutdown() {
	}
}
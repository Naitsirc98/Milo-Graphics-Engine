#include "milo/time/Time.h"
#include <GLFW/glfw3.h>

namespace milo {

	float Time::s_RawDeltaTime = 0.0f;
	float Time::s_DeltaTime = 0.0f;
	size_t Time::s_Frame = 0;

	float Time::now() noexcept {
		return static_cast<float>(glfwGetTime());
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
}
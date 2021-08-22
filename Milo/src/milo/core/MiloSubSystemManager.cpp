#pragma once

#include "milo/core/MiloSubSystemManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/Graphics.h"
#include "milo/input/Input.h"

namespace milo {

	void MiloSubSystemManager::init() {
		MemoryTracker::init();
		Log::init();
		Time::init();
		EventSystem::init();
		Graphics::init();
		Input::init();
		SceneManager::init();
	}

	void MiloSubSystemManager::shutdown() {
		SceneManager::shutdown();
		Input::shutdown();
		Graphics::shutdown();
		EventSystem::shutdown();
		MemoryTracker::shutdown();
	}
}

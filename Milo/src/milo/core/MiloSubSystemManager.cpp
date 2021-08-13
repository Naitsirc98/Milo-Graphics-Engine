#pragma once

#include "milo/core/MiloSubSystemManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	void MiloSubSystemManager::init() {
		MemoryTracker::s_Active = true;
		Log::init();
		Time::init();
		EventSystem::init();
		Graphics::init();
		SceneManager::init();
	}

	void MiloSubSystemManager::shutdown() {
		SceneManager::shutdown();
		Graphics::shutdown();
		EventSystem::shutdown();
		MemoryTracker::s_Active = false;
	}
}

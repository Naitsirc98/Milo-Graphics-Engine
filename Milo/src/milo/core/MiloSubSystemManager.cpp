#pragma once

#include "milo/core/MiloSubSystemManager.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	void MiloSubSystemManager::init() {
		Log::init();
		Time::init();
		EventSystem::init();
		SceneManager::init();
	}

	void MiloSubSystemManager::shutdown() {
		SceneManager::shutdown();
		EventSystem::shutdown();
	}
}

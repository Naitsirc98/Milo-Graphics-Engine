#pragma once

#include "milo/core/MiloSubSystemManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	void MiloSubSystemManager::init() {
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
	}
}

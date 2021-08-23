#pragma once

#include "milo/core/MiloSubSystemManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/Graphics.h"
#include "milo/input/Input.h"
#include "milo/assets/AssetManager.h"

#define INIT(system) Log::info("Initializing {}...", #system); system::init(); Log::info("{} initialized", #system)
#define SHUTDOWN(system) Log::info("Terminating {}...", #system); system::shutdown(); Log::info("{} terminated", #system)

namespace milo {

	void MiloSubSystemManager::init() {
		MemoryTracker::init();
		Log::init();
		INIT(Time);
		INIT(EventSystem);
		INIT(Graphics);
		INIT(Input);
		INIT(SceneManager);
		INIT(AssetManager);
	}

	void MiloSubSystemManager::shutdown() {
		SHUTDOWN(AssetManager);
		SHUTDOWN(SceneManager);
		SHUTDOWN(Input);
		SHUTDOWN(Graphics);
		SHUTDOWN(EventSystem);
		SHUTDOWN(Time);
		Log::shutdown();
		MemoryTracker::shutdown();
	}
}

#pragma once

#include "milo/core/MiloSubSystemManager.h"

namespace milo {

	void MiloSubSystemManager::init() {
		Log::init();
		Time::init();
		EventSystem::init();
	}

	void MiloSubSystemManager::shutdown() {
		EventSystem::shutdown();
	}
}

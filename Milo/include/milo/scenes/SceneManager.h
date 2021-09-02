#pragma once

#include "Scene.h"

namespace milo {

	class SceneManager {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static Scene* s_ActiveScene;
	public:
		static Scene* activeScene();
		// TODO: load and save scenes
	private:
		static void update();
		static void lateUpdate();

		static void init();
		static void shutdown();
	};
}
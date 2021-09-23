#include "milo/scenes/SceneManager.h"
#include "milo/time/Profiler.h"

namespace milo {

	Scene* SceneManager::s_ActiveScene = nullptr;

	Scene* SceneManager::activeScene() {
		return s_ActiveScene;
	}

	void SceneManager::update() {
		MILO_PROFILE_FUNCTION;
		s_ActiveScene->update();
	}

	void SceneManager::lateUpdate() {
		s_ActiveScene->lateUpdate();
	}

	void SceneManager::init() {
		// TODO
		s_ActiveScene = new Scene("Default Scene");
	}

	void SceneManager::shutdown() {
		DELETE_PTR(s_ActiveScene);
	}
}
#include "milo/scenes/SceneManager.h"

namespace milo {

	Scene* SceneManager::s_ActiveScene = nullptr;

	Scene *SceneManager::activeScene() {
		return s_ActiveScene;
	}

	void SceneManager::update() {
		s_ActiveScene->update();
	}

	void SceneManager::lateUpdate() {
		s_ActiveScene->lateUpdate();
	}

	void SceneManager::render() {
		s_ActiveScene->render();
	}

	void SceneManager::renderUI() {
		s_ActiveScene->renderUI();
	}

	void SceneManager::init() {
		// TODO: debug purposes
		s_ActiveScene = new Scene("Default Scene");
	}

	void SceneManager::shutdown() {
		DELETE_PTR(s_ActiveScene);
	}
}
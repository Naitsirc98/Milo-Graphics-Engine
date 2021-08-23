#include "milo/scenes/SceneManager.h"
#include "milo/scenes/components/CameraController.h"
#include "milo/scenes/Entity.h"
#include "milo/input/Input.h"
#include "milo/graphics/Window.h"

namespace milo {

	void CameraController::onLateUpdate(EntityId entityId) {

		Scene* scene = SceneManager::activeScene();
		Entity cameraEntity = scene->cameraEntity();

		if(!cameraEntity.valid()) return;

		Transform& transform = cameraEntity.getComponent<Transform>();
		Camera& camera = cameraEntity.getComponent<Camera>();

		handleMovement(transform, camera);
		handleDirection(transform, camera);
	}

	void CameraController::handleMovement(Transform& transform, Camera& camera) {

		float speed = 3 * Time::deltaTime();
		if(Input::isKeyActive(Key::Key_Left_Shift)) speed *= 2;
		if(Input::isKeyActive(Key::Key_Left_Alt)) speed /= 2;

		// FORWARD
		if(Input::isKeyActive(Key::Key_W)) {
			transform.translation += camera.forward() * speed;
		}
		// BACKWARD
		if(Input::isKeyActive(Key::Key_S)) {
			transform.translation += -camera.forward() * speed;
		}
		// LEFT
		if(Input::isKeyActive(Key::Key_A)) {
			transform.translation += -camera.right() * speed;
		}
		// RIGHT
		if(Input::isKeyActive(Key::Key_D)) {
			transform.translation += camera.right() * speed;
		}
		// UP
		if(Input::isKeyActive(Key::Key_Space)) {
			transform.translation += camera.up() * speed;
		}
		// DOWN
		if(Input::isKeyActive(Key::Key_Left_Control)) {
			transform.translation += -camera.up() * speed;
		}

		if(Input::isKeyTyped(Key::Key_P)) Log::info("Camera position = " + str(transform.translation));
	}

	void CameraController::handleDirection(Transform& transform, Camera& camera) {
		if(Window::get()->cursorMode() == CursorMode::Captured) {
			camera.lookAt(Input::getMousePosition());
			camera.zoom(Input::getMouseScroll().y);
		}
	}
}
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
		if(!scene->focused()) return;

		if(Input::isMouseButtonPressed(MouseButton::Mouse_Button_2)) {
			Window::get()->cursorMode(CursorMode::Captured);
		} else {
			Window::get()->cursorMode(CursorMode::Normal);
			return;
		}

		Transform& transform = cameraEntity.getComponent<Transform>();
		Camera& camera = cameraEntity.getComponent<Camera>();

		handleMovement(transform, camera);
		handleDirection(transform, camera);

		if(Time::seconds() - m_LogPosLastTime > 3) {
			Log::debug("Camera: pos = {}, dir = {}", str(transform.translation()), str(camera.forward()));
			m_LogPosLastTime = Time::seconds();
		}
	}

	void CameraController::handleMovement(Transform& transform, Camera& camera) {

		float speed = 3 * Time::deltaTime();
		if(Input::isKeyActive(Key::Key_Left_Shift)) speed *= 2;
		if(Input::isKeyActive(Key::Key_Left_Alt)) speed /= 2;

		Vector3 translation = transform.translation();

		// FORWARD
		if(Input::isKeyActive(Key::Key_W)) {
			translation += camera.forward() * speed;
		}
		// BACKWARD
		if(Input::isKeyActive(Key::Key_S)) {
			translation += -camera.forward() * speed;
		}
		// LEFT
		if(Input::isKeyActive(Key::Key_A)) {
			translation += -camera.right() * speed;
		}
		// RIGHT
		if(Input::isKeyActive(Key::Key_D)) {
			translation += camera.right() * speed;
		}
		// UP
		if(Input::isKeyActive(Key::Key_Space)) {
			translation += camera.up() * speed;
		}
		// DOWN
		if(Input::isKeyActive(Key::Key_Left_Control)) {
			translation += -camera.up() * speed;
		}

		if(Input::isKeyTyped(Key::Key_P)) Log::info("Camera position = " + str(translation));

		transform.translation(translation);
	}

	void CameraController::handleDirection(Transform& transform, Camera& camera) {

		if(Window::get()->cursorMode() == CursorMode::Captured) {
			camera.lookAt(Input::getMousePosition());
			camera.zoom(Input::getMouseScroll().y);
		}
	}
}
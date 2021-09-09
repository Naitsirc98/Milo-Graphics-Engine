#include "milo/editor/EditorCamera.h"
#include "milo/input/Input.h"
#include "milo/editor/UIRenderer.h"
#include "milo/events/EventSystem.h"
#include "milo/scenes/SceneManager.h"

#define M_PI 3.14159f

namespace milo {

	EditorCamera::EditorCamera() {

		EventSystem::addEventCallback(EventType::MouseScroll, [this](const Event& e) {
			if(!m_Active) return;

			const auto& event = (const MouseScrollEvent&) e;

			if(Input::isMouseButtonPressed(MouseButton::Mouse_Button_Right)) {
				if(event.offset.y > 0) m_Speed += m_Speed * 0.3f;
				else m_Speed -= m_Speed * 0.3f;
			} else {
				zoom(event.offset.y + 0.1f);
				updateCameraView();
			}
		});

		EventSystem::addEventCallback(EventType::KeyRelease, [this](const Event& e) {
			if(!m_Active) return;

			const auto& event = (const KeyReleaseEvent&) e;

			if(event.key == Key::Key_Left_Shift || event.key == Key::Key_Left_Control) {
				if(m_LastSpeed != 0.0f) {
					m_Speed = m_LastSpeed;
					m_LastSpeed = 0.0f;
				}
				m_Speed = clamp(m_Speed, 0.0005f, 2.0f);
			}
		});

		EventSystem::addEventCallback(EventType::KeyPress, [this](const Event& e) {
			if(!m_Active) return;

			const auto& event = (const KeyPressEvent&) e;

			if(m_LastSpeed == 0.0f) {
				if(event.key == Key::Key_Left_Shift) {
					m_LastSpeed = m_Speed;
					m_Speed *= 2.0f - log(m_Speed);
				} else if(event.key == Key::Key_Left_Control) {
					m_LastSpeed = m_Speed;
					m_Speed /= 2.0f - log(m_Speed);
				}
				m_Speed = clamp(m_Speed, 0.0005f, 2.0f);
			}
		});
	}

	EditorCamera::EditorCamera(const Matrix4 &projMatrix) : m_ProjectionMatrix(projMatrix) {

		m_FocalPoint = Vector3(0.0f);

		Vector3 position = { -5, 5, 5 };
		m_Distance = glm::distance(position, m_FocalPoint);

		m_Yaw = 3.0f * (float)M_PI / 4.0f;
		m_Pitch = M_PI / 4.0f;

		m_Position = calculatePosition();
		Quaternion orientation = this->orientation();
		m_WorldRotation = milo::eulerAngles(orientation) * (180.0f / (float)M_PI);
		m_ViewMatrix = milo::translate(milo::Matrix4(1.0f), m_Position) * milo::toMat4(orientation);
		m_ViewMatrix = milo::inverse(m_ViewMatrix);
	}

	EditorCamera::~EditorCamera() = default;

	static void disableMouse() {
		Input::setCursorMode(CursorMode::Captured);
		UI::setMouseEnabled(false);
	}

	static void enableMouse() {
		Input::setCursorMode(CursorMode::Normal);
		UI::setMouseEnabled(true);
	}

	void EditorCamera::update() {

		setActive(SceneManager::activeScene()->focused());

		const Vector2 mouse = Input::getMousePosition();
		const Vector2 delta = (mouse - m_InitialMousePosition) * 0.002f;

		const float ts = Time::deltaTime() * 1000;

		if (isActive()) {
			handleInput(delta, ts);
		}
		m_InitialMousePosition = mouse;

		m_Position += m_PositionDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		if (m_CameraMode == Mode::ArcBall)
			m_Position = calculatePosition();

		updateCameraView();
	}

	void EditorCamera::handleInput(const Vector2 &delta, const float ts) {

		if (Input::isMouseButtonPressed(MouseButton::Mouse_Button_Right) && !Input::isKeyActive(Key::Key_Left_Alt)) {
			m_CameraMode = Mode::FreeCam;
			disableMouse();
			const float yawSign = up().y < 0 ? 1.0f : -1.0f;

			if (Input::isKeyActive(Key::Key_Q))
				m_PositionDelta -= ts * m_Speed * Vector3{0.f, yawSign, 0.f };
			if (Input::isKeyActive(Key::Key_E))
				m_PositionDelta += ts * m_Speed * Vector3{0.f, yawSign, 0.f };
			if (Input::isKeyActive(Key::Key_S))
				m_PositionDelta -= ts * m_Speed * m_WorldRotation;
			if (Input::isKeyActive(Key::Key_W))
				m_PositionDelta += ts * m_Speed * m_WorldRotation;
			if (Input::isKeyActive(Key::Key_A))
				m_PositionDelta += ts * m_Speed * m_RightDirection;
			if (Input::isKeyActive(Key::Key_D))
				m_PositionDelta -= ts * m_Speed * m_RightDirection;

			constexpr float maxRate{ 0.12f };
			float rotationSpeed = EditorCamera::rotationSpeed();
			m_YawDelta += clamp(yawSign * -delta.x * rotationSpeed, -maxRate, maxRate);
			m_PitchDelta += clamp(-delta.y * rotationSpeed, -maxRate, maxRate);

			m_RightDirection = cross(m_WorldRotation, vec3{0.0f, yawSign, 0.0f });

			m_WorldRotation = rotate(normalize(cross(angleAxis(-m_PitchDelta, m_RightDirection),
													 angleAxis(-m_YawDelta, Vector3{0.0f, yawSign, 0.0f }))),
									 m_WorldRotation);

		} else if (Input::isKeyActive(Key::Key_Left_Alt)) {

			m_CameraMode = Mode::ArcBall;

			if (Input::isMouseButtonPressed(MouseButton::Mouse_Button_Middle)) {
				disableMouse();
				mousePan(delta);
			} else if (Input::isMouseButtonPressed(MouseButton::Mouse_Button_Left)) {
				disableMouse();
				mouseRotate(delta);
			} else if (Input::isMouseButtonPressed(MouseButton::Mouse_Button_Right)) {
				disableMouse();
				zoom(delta.x + delta.y);
			} else
				enableMouse();
		} else {
			enableMouse();
		}
	}

	void EditorCamera::focus(const Vector3 &focusPoint) {
		m_FocalPoint = focusPoint;
		if (m_Distance > m_MinFocusDistance) {
			const float distance = m_Distance - m_MinFocusDistance;
			zoom(distance / zoomSpeed());
			m_CameraMode = Mode::ArcBall;
		}
		m_Position = m_FocalPoint - forward() * m_Distance;
		updateCameraView();
	}

	const Matrix4& EditorCamera::viewMatrix() const {
		return m_ViewMatrix;
	}

	const Matrix4& EditorCamera::projMatrix() const {
		return m_ProjectionMatrix;
	}

	Matrix4 EditorCamera::projViewMatrix() const {
		return m_ProjectionMatrix * m_ViewMatrix;
	}

	bool EditorCamera::isActive() const {
		return m_Active;
	}

	void EditorCamera::setActive(bool active) {
		m_Active = active;
	}

	float EditorCamera::distance() const {
		return m_Distance;
	}

	void EditorCamera::setDistance(float distance) {
		m_Distance = distance;
	}

	const Vector3& EditorCamera::focalPoint() const {
		return m_FocalPoint;
	}

	void EditorCamera::setViewportSize(uint32_t width, uint32_t height) {
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

	Vector3 EditorCamera::up() const {
		return rotate(orientation(), Vector3(0, 1, 0));
	}

	Vector3 EditorCamera::right() const {
		return rotate(orientation(), Vector3(1, 0, 0));
	}

	Vector3 EditorCamera::forward() const {
		return rotate(orientation(), Vector3(0, 0, 1));
	}

	const Vector3 &EditorCamera::position() const {
		return m_Position;
	}

	void EditorCamera::setPosition(const Vector3& position) {
		m_Position = position;
		m_PositionDelta = {0, 0, 0};
	}

	Quaternion EditorCamera::orientation() const {
		return Quaternion(Vector3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f));
	}

	float EditorCamera::pitch() const {
		return m_Pitch;
	}

	float EditorCamera::yaw() const {
		return m_Yaw;
	}

	float EditorCamera::speed() const {
		return m_Speed;
	}

	void EditorCamera::setSpeed(float speed) {
		m_Speed = speed;
	}

	void EditorCamera::updateCameraView() {

		const float yawSign = up().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const Vector3& forward = this->forward();
		const float cosAngle = dot(forward, up());
		if (cosAngle * yawSign > 0.99f)
			m_PitchDelta = 0.0f;

		Vector3 lookAt = m_Position + forward;
		m_WorldRotation = milo::normalize(m_FocalPoint - m_Position);
		m_FocalPoint = m_Position + forward * m_Distance;
		m_Distance = milo::distance(m_Position, m_FocalPoint);
		m_ViewMatrix = milo::lookAt(m_Position, lookAt, { 0.0f, yawSign, 0.0f });

		// Damping for smooth camera
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_PositionDelta *= 0.8f;
	}

	void EditorCamera::mousePan(const Vector2 &delta) {
		auto [xSpeed, ySpeed] = panSpeed();
		m_FocalPoint += -right() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += up() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::mouseRotate(const Vector2 &delta) {
		const float yawSign = up().y < 0.0f ? -1.0f : 1.0f;
		m_YawDelta += yawSign * delta.x * rotationSpeed();
		m_PitchDelta += delta.y * rotationSpeed();
	}

	void EditorCamera::zoom(float delta) {
		m_Distance -= delta * zoomSpeed();
		m_Position = m_FocalPoint - forward() * m_Distance;
		const glm::vec3 forwardDir = forward();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += forwardDir;
			m_Distance = 1.0f;
		}
		m_PositionDelta += delta * zoomSpeed() * forwardDir;
	}

	Vector3 EditorCamera::calculatePosition() const {
		return m_FocalPoint - forward() * m_Distance + m_PositionDelta;
	}

	Pair<float, float> EditorCamera::panSpeed() const {

		const float x = std::min(float(m_ViewportWidth) / 1000.0f, 2.4f);
		const float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		const float y = std::min(float(m_ViewportHeight) / 1000.0f, 2.4f);
		const float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::rotationSpeed() const {
		return 0.3f;
	}

	float EditorCamera::zoomSpeed() const {
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		return std::min(distance * distance, 100.0f);
	}

}

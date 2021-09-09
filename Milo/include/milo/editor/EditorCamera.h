// Based on https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Renderer/EditorCamera.h

#pragma once

#include "milo/common/Common.h"

namespace milo {

	class EditorCamera {
		friend class MiloEditor;
	public:
		enum class Mode {
			Undefined, FreeCam, ArcBall
		};
	private:
		Matrix4 m_ViewMatrix = Matrix4(1.0f);
		Matrix4 m_ProjectionMatrix = Matrix4(1.0f);

		float m_Exposure{0.8f};

		Vector3 m_Position;
		Vector3 m_WorldRotation;
		Vector3 m_FocalPoint;

		bool m_Active = false;
		bool m_Panning, m_Rotating;
		Vector2 m_InitialMousePosition {};
		Vector3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_Speed {0.002f};
		float m_LastSpeed = 0.f;

		float m_Pitch, m_Yaw;
		float m_PitchDelta{}, m_YawDelta{};
		Vector3 m_PositionDelta{};
		Vector3 m_RightDirection {};

		Mode m_CameraMode { Mode::ArcBall };

		float m_MinFocusDistance = 100.0f;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

	public:
		EditorCamera();
		EditorCamera(const Matrix4& projMatrix);
		~EditorCamera();

	public:

		void update();

		void focus(const Vector3& focusPoint);

		const Matrix4& viewMatrix() const;
		const Matrix4& projMatrix() const;
		Matrix4 projViewMatrix() const;

		bool isActive() const;
		void setActive(bool active);

		float distance() const;
		void setDistance(float distance);

		const Vector3& focalPoint() const;

		void setViewportSize(uint32_t width, uint32_t height);

		Vector3 up() const;
		Vector3 right() const;
		Vector3 forward() const;

		const Vector3& position() const;

		Quaternion orientation() const;

		float pitch() const;
		float yaw() const;

		float speed() const;
		void setSpeed(float speed);

	private:

		void updateCameraView();

		void mousePan(const Vector2& delta);
		void mouseRotate(const Vector2& delta);
		void zoom(float delta);

		Vector3 calculatePosition() const;

		Pair<float, float> panSpeed() const;
		float rotationSpeed() const;
		float zoomSpeed() const;

		void handleInput(const Vector2 &delta, const float ts);
	};
}
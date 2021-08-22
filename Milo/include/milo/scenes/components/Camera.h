#pragma once

#include "milo/common/Common.h"

namespace milo {

	enum class ProjectionType {
		Perspective, Orthographic
	};

	class Camera {
	public:
		inline static const float DEFAULT_MAX_FOV = radians(90.0f);
		inline static const float DEFAULT_MIN_FOV = radians(1.0f);
		inline static const float MIN_PITCH = -89.0f;
		inline static const float MAX_PITCH = 89.0f;
		inline static const float DEFAULT_YAW = -90.0f;
		inline static const float DEFAULT_NEAR_PLANE = 0.1f;
		inline static const float DEFAULT_FAR_PLANE = 1000.0f;
		inline static const float DEFAULT_EXPOSURE = 1.0f;
	private:
		// Axis
		mutable Vector3 m_Forward = {0, 0, -1};
		mutable Vector3 m_Up = {0, 1, 0};
		mutable Vector3 m_Right = {1, 0, 0};
		// Viewport
		Vector4 m_Viewport = {0, 0, 0, 0};
		// Projection information
		ProjectionType m_ProjectionType = ProjectionType::Perspective;
		float m_OrthographicSize = 10.0f;
		// Field of view
		Range<float> m_FovRange = {DEFAULT_MIN_FOV, DEFAULT_MAX_FOV};
		float m_Fov = clamp(m_FovRange.max / 2.0f, m_FovRange.min, m_FovRange.max);
		// Rotation angles
		float m_Yaw = DEFAULT_YAW;
		float m_Pitch = 0;
		float m_Roll = 0;
		// Planes
		float m_NearPlane = DEFAULT_NEAR_PLANE;
		float m_FarPlane = DEFAULT_FAR_PLANE;
		// HDR
		float m_Exposure = DEFAULT_EXPOSURE;
		// Movement
		Vector2 m_LastPosition = {0, 0};
		// Rendering
		Vector4 m_ClearColor = {0.15f, 0.15f, 0.15f, 1};
	public:
		Camera() = default;
		~Camera() = default;
		inline ProjectionType projectionType() const { return m_ProjectionType;}
		inline Camera& projectionType(ProjectionType projectionType) {m_ProjectionType = projectionType; return *this;}
		inline float orthographicSize() const { return m_OrthographicSize;}
		inline Camera& orthographicSize(float orthographicSize) {m_OrthographicSize = orthographicSize; return *this;}
		inline const Vector4& viewport() const {return m_Viewport;}
		inline Camera& viewport(const Vector4& viewport) {m_Viewport = viewport; return *this;}
		inline const Vector3& forward() const {return m_Forward;}
		inline const Vector3& up() const {return m_Up;}
		inline const Vector3& right() const {return m_Right;}
		inline const Range<float>& fovRange() const {return m_FovRange;}
		inline Camera& fovRange(const Range<float>& fovRange) {m_FovRange = fovRange; return *this;}
		inline float fov() const {return m_Fov;}
		inline Camera& fov(float fov) {m_Fov = fov; return *this;}
		inline float yaw() const {return m_Yaw;}
		inline Camera& yaw(float yaw) {m_Yaw = yaw; return *this;}
		inline float pitch() const {return m_Pitch;}
		inline Camera& pitch(float pitch) {m_Pitch = pitch; return *this;}
		inline float roll() const {return m_Roll;}
		inline Camera& roll(float roll) {m_Roll = roll; return *this;}
		inline float nearPlane() const {return m_NearPlane;}
		inline Camera& nearPlane(float nearPlane) {m_NearPlane = nearPlane; return *this;}
		inline float farPlane() const {return m_FarPlane;}
		inline Camera& farPlane(float farPlane) {m_FarPlane = farPlane; return *this;}
		inline float exposure() const {return m_Exposure;}
		inline Camera& exposure(float exposure) {m_Exposure = exposure; return *this;}
		inline const Vector4& clearColor() const {return m_ClearColor;}
		inline Camera& clearColor(const Vector4& clearColor) {m_ClearColor = clearColor; return *this;}

		inline Camera& zoom(float zoom) {
			fov(m_Fov - radians(zoom));
			return *this;
		}

		inline Camera& lookAt(float x, float y, float sensitivity = 1.0f) {
			return lookAt({x, y}, sensitivity);
		}

		inline Camera& lookAt(const Vector2& position, float sensitivity = 1.0f) {

			if(position == m_LastPosition) return *this;

			float xOffset = position.x - m_LastPosition.x;
			float yOffset = m_LastPosition.y - position.y;

			m_LastPosition = position;

			float fov = max(this->fov(), 1.0f);

			xOffset *= sensitivity / (m_FovRange.max / fov);
			yOffset *= sensitivity / (m_FovRange.max / fov);

			m_Yaw += xOffset;
			m_Pitch = clamp(yOffset + m_Pitch, MIN_PITCH, MAX_PITCH);
		}

		inline const Matrix4 viewMatrix(const Matrix4& transform) const { return inverse(transform);} // TODO

		inline const Matrix4 projectionMatrix() const {

			updateOrientation();

			float aspect = m_Viewport.w == 0 ? 0 : m_Viewport.z / m_Viewport.w;

			if(m_ProjectionType == ProjectionType::Perspective) {
				return perspective(m_Fov, aspect, m_NearPlane, m_FarPlane);
			}

			float left =   -m_OrthographicSize * aspect * 0.5f;
			float right =   m_OrthographicSize * aspect * 0.5f;
			float bottom = -m_OrthographicSize * 0.5f;
			float top =     m_OrthographicSize * 0.5f;

			return milo::ortho(left, right, bottom, top, m_NearPlane, m_FarPlane);
		}

		inline void updateOrientation() const noexcept {
			const float yaw = radians(m_Yaw);
			const float pitch = radians(m_Pitch);
			m_Forward.x = cos(yaw) * cos(pitch);
			m_Forward.y = sin(pitch);
			m_Forward.z = sin(yaw) * cos(pitch);
			m_Forward = normalize(m_Forward);
			m_Right = normalize(cross(m_Forward, {0, 1, 0}));
			m_Up = normalize(cross(m_Right, m_Forward));
		}
	};
}
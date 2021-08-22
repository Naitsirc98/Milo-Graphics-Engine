#pragma once

#include "milo/common/Common.h"

namespace milo {

	struct Camera {
		enum class Type { Perspective, Orthographic };

		Type type = Type::Perspective;
		float aspectRatio = 0.0f;

		struct PerspectiveState {
			float fov = milo::radians(45.0f);
			float zNear = 0.01f;
			float zFar = 1000.0f;
		} perspective;

		struct OrthographicState {
			float size = 10.0f;
			float zNear = -1.0f;
			float zFar = 1.0f;
		} orthographic;

		inline Matrix4 projectionMatrix() const {

			if(type == Type::Perspective) return milo::perspective(perspective.fov, aspectRatio, perspective.zNear, perspective.zFar);

			float left =   -orthographic.size * aspectRatio * 0.5f;
			float right =   orthographic.size * aspectRatio * 0.5f;
			float bottom = -orthographic.size * 0.5f;
			float top =     orthographic.size * 0.5f;

			return milo::ortho(left, right, bottom, top, orthographic.zNear, orthographic.zFar);
		}

		inline Matrix4 viewMatrix(const Vector3& position, const Vector3& rotation) const {
			// TODO
			return Matrix4();
		}
	};
}
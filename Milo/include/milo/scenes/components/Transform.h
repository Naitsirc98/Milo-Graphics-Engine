#pragma once

#include "milo/math/Math.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace milo {

	struct Transform {

		Vector3 translation;
		Vector3 scale;
		Quaternion rotation;

	public:
		Transform() : translation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f) {}

		inline void rotate(float radians, const Vector3 axis) {
			rotation = angleAxis(radians, axis);
		}

		inline Matrix4 modelMatrix() const noexcept {
			return milo::translate(translation) * milo::scale(scale) * milo::toMat4(rotation);
		}

		inline void setMatrix(const Matrix4& matrix) {
			Vector3 skew;
			Vector4 perspective;
			milo::decompose(matrix, scale, rotation, translation, skew, perspective);
		}
	};
}
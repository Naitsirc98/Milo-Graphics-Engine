#pragma once

#include "milo/math/Math.h"

namespace milo {

	struct Transform {
		Vector3 translation;
		Vector3 scale;
		Quaternion rotation;

		Transform() : translation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), rotation(0.0f, 0.0f, 0.0f, 1.0f) {}

		[[nodiscard]] inline Matrix4 modelMatrix() const noexcept {
			return milo::translate(translation) * milo::scale(scale) * milo::toMat4(rotation);
		}
	};
}
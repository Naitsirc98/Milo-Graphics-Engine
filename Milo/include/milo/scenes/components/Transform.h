#pragma once

#include "milo/math/Math.h"

namespace milo {

	struct Transform {
		Vector3 translation;
		Vector3 scale;
		Quaternion rotation;

		[[nodiscard]] inline Matrix4 modelMatrix() const noexcept {
			return milo::translate(translation) * milo::scale(scale) * milo::toMat4(rotation);
		}
	};
}
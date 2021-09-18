#pragma once

#include "milo/common/Common.h"
#include "milo/assets/skybox/Skybox.h"

namespace milo {

	struct DirectionalLight {

		Vector3 direction = { 0.0f, 0.0f, 0.0f };
		float _padding0;
		Vector3 color = { 1, 1, 1 };
		float multiplier = 1.0f;

		bool castShadows = true;
		Vector3 _padding1;
	};

	struct SkyLight {

		PreethamSky* sky{nullptr};
		DirectionalLight light;
	};

	struct PointLight {

		// Get position from entity's transform
		Vector3 position = {0, 0, 0};
		Vector3 color = { 1, 1, 1 };
		float multiplier = 1.0f;
		float minRadius = 0.001f;
		float radius = 25.0f;
		float falloff = 1.0f;
		float sourceSize = 0.1f;
		bool castsShadows = true;
		byte_t padding[3]{0};
	};
}
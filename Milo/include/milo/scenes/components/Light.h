#pragma once

#include "milo/common/Common.h"
#include "milo/assets/skybox/Skybox.h"

namespace milo {

	struct DirectionalLight {

		Vector3 direction = { 0.0f, 0.0f, 0.0f };
		Vector3 radiance = { 0.0f, 0.0f, 0.0f };
		float multiplier = 0.0f;

		bool castShadows = true;
	};

	struct SkyLight {

		PreethamSky* sky{nullptr};
		DirectionalLight light;
	};

	struct PointLight {

		// Get position from entity's transform
		Vector3 position = {0, 0, 0};
		Vector3 radiance = { 0.0f, 0.0f, 0.0f };
		float multiplier = 0.0f;
		float minRadius = 0.001f;
		float radius = 25.0f;
		float falloff = 1.f;
		float sourceSize = 0.1f;
		bool castsShadows = true;
		byte_t padding[3]{0};
	};
}
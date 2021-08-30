#pragma once

#include "Skybox.h"

namespace milo {

	struct SkyboxLoadInfo {
		int environmentMapSize{2048};
		int irradianceMapSize{32};
		int prefilterMapSize{128};
		int brdfSize{512};
		float maxLOD{4.0f};
		float lodBias{-0.5f};
	};

	const SkyboxLoadInfo DEFAULT_SKYBOX_LOAD_INFO = {};

	class SkyboxFactory {
	public:
		virtual ~SkyboxFactory() = default;
		virtual Skybox* create(const String& name, const String& imageFile, const SkyboxLoadInfo& loadInfo = DEFAULT_SKYBOX_LOAD_INFO) = 0;
	public:
		static SkyboxFactory* create();
	};
}
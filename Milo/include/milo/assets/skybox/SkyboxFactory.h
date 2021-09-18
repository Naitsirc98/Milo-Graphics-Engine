#pragma once

#include "Skybox.h"

namespace milo {

	struct SkyboxLoadInfo {
		uint32_t environmentMapSize{2048};
		uint32_t irradianceMapSize{64};
		uint32_t prefilterMapSize{512};
		uint32_t brdfSize{512};
		float maxLOD{4.0f};
		float lodBias{-0.5f};
	};

	const SkyboxLoadInfo DEFAULT_SKYBOX_LOAD_INFO = {};

	class SkyboxFactory {
	public:
		virtual ~SkyboxFactory() = default;
		virtual Skybox* create(const String& name, const String& imageFile, const SkyboxLoadInfo& loadInfo = DEFAULT_SKYBOX_LOAD_INFO) = 0;
		virtual PreethamSky* createPreethamSky(const String& name, const SkyboxLoadInfo& loadInfo, float turbidity, float azimuth, float inclination) = 0;
		virtual void updatePreethamSky(PreethamSky* sky) = 0;
	public:
		static SkyboxFactory* create();
	};
}
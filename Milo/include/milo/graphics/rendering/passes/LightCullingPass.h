#pragma once

#include "RenderPass.h"
#include "milo/scenes/components/Light.h"

namespace milo {

	struct VisibleLightsBuffer {
		uint32_t indices[MAX_POINT_LIGHTS];
	};

	constexpr uint32_t TILE_SIZE = 16;

	class LightCullingPass : public RenderPass {
	public:
		LightCullingPass() = default;
		virtual ~LightCullingPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static LightCullingPass* create();
		static size_t id();
		static Handle getVisibleLightsBufferHandle(uint32_t index = UINT32_MAX);
	};
}
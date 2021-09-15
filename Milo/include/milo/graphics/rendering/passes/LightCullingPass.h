#pragma once

#include "RenderPass.h"

namespace milo {

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
		static Handle getVisibleLightsBufferHandle();
	};
}
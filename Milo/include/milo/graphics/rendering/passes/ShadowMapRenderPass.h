#pragma once

#include "RenderPass.h"

namespace milo {

	constexpr uint32_t MAX_SHADOW_CASCADES = 4;

	class ShadowMapRenderPass : public RenderPass {
	public:
		ShadowMapRenderPass() = default;
		virtual ~ShadowMapRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static ShadowMapRenderPass* create();
		static size_t id();
		static Handle getCascadeShadowMap(uint32_t cascadeIndex, uint32_t index = UINT32_MAX);
	};
}


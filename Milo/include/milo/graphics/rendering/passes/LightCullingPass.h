#pragma once

#include "RenderPass.h"

namespace milo {

	class LightCullingPass : public RenderPass {
	public:
		LightCullingPass() = default;
		virtual ~LightCullingPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static LightCullingPass* create();
		static size_t id();
	};
}
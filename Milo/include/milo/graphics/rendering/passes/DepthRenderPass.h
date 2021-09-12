#pragma once

#include "RenderPass.h"

namespace milo {

	class DepthRenderPass : public RenderPass {
	public:
		DepthRenderPass() = default;
		virtual ~DepthRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static DepthRenderPass* create();
		static size_t id();
	};
}
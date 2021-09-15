#pragma once

#include "RenderPass.h"

namespace milo {

	class PreDepthRenderPass : public RenderPass {
	public:
		PreDepthRenderPass() = default;
		virtual ~PreDepthRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static PreDepthRenderPass* create();
		static size_t id();
		static Handle getFramebufferHandle();
	protected:
		static Handle createFramebufferHandle(uint32_t index);
	};
}
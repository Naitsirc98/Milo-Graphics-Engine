#pragma once

#include "RenderPass.h"

namespace milo {

	class FinalRenderPass : public RenderPass {
	public:
		virtual ~FinalRenderPass() = default;
		RenderPassId getId() const override;
		const String& name() const override;

		bool shouldCompile(Scene *scene) const override;

	public:
		static FinalRenderPass* create();
		static size_t id();
	};

}
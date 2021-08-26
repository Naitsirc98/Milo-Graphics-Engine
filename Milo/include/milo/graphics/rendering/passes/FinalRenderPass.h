#pragma once

#include "RenderPass.h"

namespace milo {

	class FinalRenderPass : public RenderPass {
	public:
		virtual ~FinalRenderPass() = default;
		InputDescription inputDescription() const override;
		OutputDescription outputDescription() const override;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static FinalRenderPass* create();
		static size_t id();
	};

}
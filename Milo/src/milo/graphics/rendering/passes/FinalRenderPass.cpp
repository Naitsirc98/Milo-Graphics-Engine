#include "milo/graphics/rendering/passes/FinalRenderPass.h"

namespace milo {

	RenderPass::InputDescription FinalRenderPass::inputDescription() const {
		return InputDescription();
	}

	RenderPass::OutputDescription FinalRenderPass::outputDescription() const {
		return OutputDescription();
	}

	FinalRenderPass* FinalRenderPass::create() {
		return nullptr; // TODO: API specific
	}

	size_t FinalRenderPass::id() {
		DEFINE_RENDER_PASS_ID(FinalRenderPass);
		return id;
	}
}
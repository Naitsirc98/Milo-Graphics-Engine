#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanFinalRenderPass.h"

namespace milo {

	RenderPass::InputDescription FinalRenderPass::inputDescription() const {
		return InputDescription();
	}

	RenderPass::OutputDescription FinalRenderPass::outputDescription() const {
		return OutputDescription();
	}

	RenderPassId FinalRenderPass::getId() const {
		return id();
	}

	FinalRenderPass* FinalRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFinalRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t FinalRenderPass::id() {
		DEFINE_RENDER_PASS_ID(FinalRenderPass);
		return id;
	}
}
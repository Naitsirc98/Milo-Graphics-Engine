#include "milo/graphics/rendering/passes/PBRForwardRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanPBRForwardRenderPass.h"

namespace milo {

	const String PBR_FORWARD_RENDER_PASS_NAME = "PBRForwardRenderPass";

	RenderPassId PBRForwardRenderPass::getId() const {
		return id();
	}

	const String& PBRForwardRenderPass::name() const {
		return PBR_FORWARD_RENDER_PASS_NAME;
	}

	PBRForwardRenderPass* PBRForwardRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanPBRForwardRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t PBRForwardRenderPass::id() {
		DEFINE_RENDER_PASS_ID(PBR_FORWARD_RENDER_PASS_NAME);
		return id;
	}
}
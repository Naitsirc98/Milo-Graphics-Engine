#include "milo/graphics/rendering/passes/DepthRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanDepthRenderPass.h"

namespace milo {

	static const String DEPTH_RENDER_PASS_NAME = "DepthRenderPass";

	RenderPassId DepthRenderPass::getId() const {
		return id();
	}

	const String& DepthRenderPass::name() const {
		return DEPTH_RENDER_PASS_NAME;
	}

	DepthRenderPass* DepthRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanDepthRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t DepthRenderPass::id() {
		DEFINE_RENDER_PASS_ID(DEPTH_RENDER_PASS_NAME);
		return id;
	}
}
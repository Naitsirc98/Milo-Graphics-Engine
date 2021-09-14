#include "milo/graphics/rendering/passes/PreDepthRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanPreDepthRenderPass.h"

namespace milo {

	static const String DEPTH_RENDER_PASS_NAME = "PreDepthRenderPass";

	RenderPassId PreDepthRenderPass::getId() const {
		return id();
	}

	const String& PreDepthRenderPass::name() const {
		return DEPTH_RENDER_PASS_NAME;
	}

	PreDepthRenderPass* PreDepthRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanPreDepthRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t PreDepthRenderPass::id() {
		DEFINE_RENDER_PASS_ID(DEPTH_RENDER_PASS_NAME);
		return id;
	}
}
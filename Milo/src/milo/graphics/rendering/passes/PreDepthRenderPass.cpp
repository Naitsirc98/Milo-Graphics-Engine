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

	Handle PreDepthRenderPass::getFramebufferHandle() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			uint32_t index = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
			return createFramebufferHandle(index);
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Handle PreDepthRenderPass::createFramebufferHandle(uint32_t index) {
		return id() + index;
	}
}
#include "milo/graphics/rendering/passes/ShadowMapRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanShadowMapRenderPass.h"

namespace milo {

	static const String NAME = "ShadowMapRenderPass";

	RenderPassId ShadowMapRenderPass::getId() const {
		return id();
	}

	const String& ShadowMapRenderPass::name() const {
		return NAME;
	}

	ShadowMapRenderPass* ShadowMapRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanShadowMapRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t ShadowMapRenderPass::id() {
		DEFINE_RENDER_PASS_ID(NAME);
		return id;
	}

	Handle ShadowMapRenderPass::getFramebufferHandle(uint32_t index) {
		if(index != UINT32_MAX) return id() + index;
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return id() + VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
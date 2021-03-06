#include "milo/graphics/rendering/passes/ShadowMapRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanShadowMapRenderPass.h"

namespace milo {

	static const String ShadowMapRenderPassName = "ShadowMapRenderPass";

	RenderPassId ShadowMapRenderPass::getId() const {
		return id();
	}

	const String& ShadowMapRenderPass::name() const {
		return ShadowMapRenderPassName;
	}

	ShadowMapRenderPass* ShadowMapRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanShadowMapRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t ShadowMapRenderPass::id() {
		DEFINE_RENDER_PASS_ID(ShadowMapRenderPassName);
		return id;
	}

	Handle ShadowMapRenderPass::getDepthMap(uint32_t cascadeIndex, uint32_t index) {
		if(index != UINT32_MAX) return id() + cascadeIndex * 1000;// + index;
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return id() + cascadeIndex * 1000;// + VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
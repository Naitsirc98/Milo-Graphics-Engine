#include "milo/graphics/rendering/passes/LightCullingPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanLightCullingPass.h"

namespace milo {

	static const String LightCullingPassName = "LightCullingPass";

	RenderPassId LightCullingPass::getId() const {
		return id();
	}

	const String& LightCullingPass::name() const {
		return LightCullingPassName;
	}

	LightCullingPass* LightCullingPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanLightCullingPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t LightCullingPass::id() {
		DEFINE_RENDER_PASS_ID(LightCullingPassName);
		return id;
	}

	Handle LightCullingPass::getVisibleLightsBufferHandle(uint32_t index) {
		if(index != UINT32_MAX) return id() + index;
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return id() + VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

}

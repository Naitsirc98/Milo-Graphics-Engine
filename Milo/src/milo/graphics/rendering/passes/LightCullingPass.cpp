#include "milo/graphics/rendering/passes/LightCullingPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanLightCullingPass.h"

namespace milo {

	static const String NAME = "LightCullingPass";

	RenderPassId LightCullingPass::getId() const {
		return id();
	}

	const String& LightCullingPass::name() const {
		return NAME;
	}

	LightCullingPass* LightCullingPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanLightCullingPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t LightCullingPass::id() {
		DEFINE_RENDER_PASS_ID(NAME);
		return id;
	}

	Handle LightCullingPass::getVisibleLightsBufferHandle() {
		return id();
	}

}

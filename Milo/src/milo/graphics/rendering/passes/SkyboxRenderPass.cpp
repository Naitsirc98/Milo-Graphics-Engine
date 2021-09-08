#include "milo/graphics/rendering/passes/SkyboxRenderPass.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanSkyboxRenderPass.h"

namespace milo {

	static const String SKYBOX_RENDER_PASS_NAME = "SkyboxRenderPass";

	RenderPassId SkyboxRenderPass::getId() const {
		return id();
	}

	const String& SkyboxRenderPass::name() const {
		return SKYBOX_RENDER_PASS_NAME;
	}

	SkyboxRenderPass* SkyboxRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanSkyboxRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t SkyboxRenderPass::id() {
		DEFINE_RENDER_PASS_ID(SKYBOX_RENDER_PASS_NAME);
		return id;
	}
}
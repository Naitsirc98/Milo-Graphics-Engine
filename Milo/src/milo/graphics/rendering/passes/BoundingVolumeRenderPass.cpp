#include "milo/graphics/rendering/passes/BoundingVolumeRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanBoundingVolumeRenderPass.h"

namespace milo {

	static const String BOUNDING_VOLUME_RENDER_PASS_NAME = "BoundingVolumeRenderPass";

	RenderPassId BoundingVolumeRenderPass::getId() const {
		return id();
	}

	const String& BoundingVolumeRenderPass::name() const {
		return BOUNDING_VOLUME_RENDER_PASS_NAME;
	}

	BoundingVolumeRenderPass* BoundingVolumeRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanBoundingVolumeRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t BoundingVolumeRenderPass::id() {
		DEFINE_RENDER_PASS_ID(BOUNDING_VOLUME_RENDER_PASS_NAME);
		return id;
	}
}
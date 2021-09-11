#include "milo/graphics/rendering/passes/GridRenderPass.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanGridRenderPass.h"

namespace milo {

	static const String GRID_RENDER_PASS_NAME = "GridRenderPass";

	RenderPassId GridRenderPass::getId() const {
		return id();
	}

	const String& GridRenderPass::name() const {
		return GRID_RENDER_PASS_NAME;
	}

	GridRenderPass* GridRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanGridRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t GridRenderPass::id() {
		DEFINE_RENDER_PASS_ID(GRID_RENDER_PASS_NAME);
		return id;
	}
}
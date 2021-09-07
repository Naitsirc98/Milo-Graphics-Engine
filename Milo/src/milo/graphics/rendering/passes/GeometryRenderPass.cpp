#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/Window.h"
#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanGeometryRenderPass.h"

namespace milo {

	RenderPassId GeometryRenderPass::getId() const {
		return id();
	}

	const String& GeometryRenderPass::name() const {
		static const String name = "GeometryRenderPass";
		return name;
	}

	GeometryRenderPass* GeometryRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanGeometryRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t GeometryRenderPass::id() {
		DEFINE_RENDER_PASS_ID(GeometryRenderPass);
		return id;
	}

	bool GeometryRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}
}
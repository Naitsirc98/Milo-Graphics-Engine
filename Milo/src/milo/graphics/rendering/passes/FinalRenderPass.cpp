#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanFinalRenderPass.h"

namespace milo {

	RenderPassId FinalRenderPass::getId() const {
		return id();
	}

	const String& FinalRenderPass::name() const {
		static const String name = "FinalRenderPass";
		return name;
	}

	FinalRenderPass* FinalRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFinalRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t FinalRenderPass::id() {
		DEFINE_RENDER_PASS_ID(FinalRenderPass);
		return id;
	}

	bool FinalRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}
}
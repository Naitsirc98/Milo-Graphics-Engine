#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanFinalRenderPass.h"

namespace milo {

	RenderPass::InputDescription FinalRenderPass::inputDescription() const {

		Size size = Window::get()->size();

		InputDescription input{};
		input.textures[0].width = size.width;
		input.textures[0].height = size.height;
		input.textures[0].format = PixelFormat::RGBA32F;
		input.textures[0].usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
		input.textureCount = 1;

		return input;
	}

	RenderPass::OutputDescription FinalRenderPass::outputDescription() const {
		return OutputDescription();
	}

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
}
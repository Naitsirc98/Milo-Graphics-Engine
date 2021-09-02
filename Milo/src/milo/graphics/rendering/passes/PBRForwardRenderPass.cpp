#include "milo/graphics/rendering/passes/PBRForwardRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanPBRForwardRenderPass.h"

namespace milo {

	const String PBR_FORWARD_RENDER_PASS_NAME = "PBRForwardRenderPass";

	RenderPass::InputDescription PBRForwardRenderPass::inputDescription() const {
		return RenderPass::InputDescription();
	}

	RenderPass::OutputDescription PBRForwardRenderPass::outputDescription() const {

		Size size = Window::get()->size();

		RenderPass::OutputDescription output = {};

		output.textures[0].usageFlags = TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT;
		output.textures[0].width = size.width;
		output.textures[0].height = size.height;
		output.textures[0].format = PixelFormat::RGBA32F;
		output.textures[0].mipLevels = 1;

		output.textures[1].usageFlags = TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT;
		output.textures[1].width = size.width;
		output.textures[1].height = size.height;
		output.textures[1].format = PixelFormat::DEPTH;
		output.textures[1].mipLevels = 1;

		output.textureCount = 2;

		return output;
	}

	RenderPassId PBRForwardRenderPass::getId() const {
		return id();
	}

	const String& PBRForwardRenderPass::name() const {
		return PBR_FORWARD_RENDER_PASS_NAME;
	}

	PBRForwardRenderPass* PBRForwardRenderPass::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanPBRForwardRenderPass();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	size_t PBRForwardRenderPass::id() {
		DEFINE_RENDER_PASS_ID(PBR_FORWARD_RENDER_PASS_NAME);
		return id;
	}
}
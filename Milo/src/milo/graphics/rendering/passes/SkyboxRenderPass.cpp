#include "milo/graphics/rendering/passes/SkyboxRenderPass.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanSkyboxRenderPass.h"

namespace milo {

	static const String SKYBOX_RENDER_PASS_NAME = "SkyboxRenderPass";

	RenderPass::InputDescription SkyboxRenderPass::inputDescription() const {

		Size size = Window::get()->size();

		InputDescription input{};

		input.textures[0].usageFlags = TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT;
		input.textures[0].width = size.width;
		input.textures[0].height = size.height;
		input.textures[0].format = PixelFormat::RGBA32F;
		input.textures[0].mipLevels = 1;

		input.textures[1].usageFlags = TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT;
		input.textures[1].width = size.width;
		input.textures[1].height = size.height;
		input.textures[1].format = PixelFormat::DEPTH;
		input.textures[1].mipLevels = 1;

		input.textureCount = 2;

		return input;
	}

	RenderPass::OutputDescription SkyboxRenderPass::outputDescription() const {
		return OutputDescription();
	}

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
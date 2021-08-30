#include "milo/graphics/rendering/passes/SkyboxRenderPass.h"

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

		return input;
	}

	RenderPass::OutputDescription SkyboxRenderPass::outputDescription() const {

		Size size = Window::get()->size();

		OutputDescription output{};

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

		return output;
	}

	RenderPassId SkyboxRenderPass::getId() const {
		return id();
	}

	const String& SkyboxRenderPass::name() const {
		return SKYBOX_RENDER_PASS_NAME;
	}

	SkyboxRenderPass* SkyboxRenderPass::create() {
		return nullptr;
	}

	size_t SkyboxRenderPass::id() {
		DEFINE_RENDER_PASS_ID(SKYBOX_RENDER_PASS_NAME);
		return id;
	}
}
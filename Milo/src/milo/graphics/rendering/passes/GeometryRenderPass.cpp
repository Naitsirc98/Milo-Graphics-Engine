#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/Window.h"
#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanGeometryRenderPass.h"

namespace milo {

	RenderPass::InputDescription GeometryRenderPass::inputDescription() const {
		return InputDescription();
	}

	RenderPass::OutputDescription GeometryRenderPass::outputDescription() const {

		Size size = Window::get()->size();

		OutputDescription output = {};

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
}
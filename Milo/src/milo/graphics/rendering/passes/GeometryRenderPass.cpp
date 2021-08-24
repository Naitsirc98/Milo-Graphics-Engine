#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/Window.h"
#include "milo/graphics/rendering/FrameGraph.h"

namespace milo {

	RenderPass::InputDescription GeometryRenderPass::inputDescription() const {
		return InputDescription();
	}

	RenderPass::OutputDescription GeometryRenderPass::outputDescription() const {

		Size size = Window::get()->size();

		OutputDescription output = {};

		output.textures[0].width = size.width;
		output.textures[0].height = size.height;
		output.textures[0].format = PixelFormat::RGBA32;
		output.textures[0].mipLevels = 1;

		output.textureCount = 1;

		return output;
	}

	GeometryRenderPass* GeometryRenderPass::create() {
		return nullptr; // TODO: return API specific GeometryRenderPass
	}

	size_t GeometryRenderPass::id() {
		DEFINE_RENDER_PASS_ID(GeometryRenderPass);
		return id;
	}
}
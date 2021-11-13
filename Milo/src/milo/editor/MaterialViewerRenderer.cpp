#include "milo/editor/MaterialViewerRenderer.h"
#include "milo/editor/VulkanMaterialViewerRenderer.h"

namespace milo {

	MaterialViewerRenderer* MaterialViewerRenderer::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanMaterialViewerRenderer();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported graphics api");
	}
}
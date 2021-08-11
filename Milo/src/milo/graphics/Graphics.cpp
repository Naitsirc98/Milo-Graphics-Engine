#include "milo/graphics/Graphics.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"

namespace milo {

	GraphicsAPI Graphics::s_GraphicsAPI = GraphicsAPI::Default;
	GraphicsContext* Graphics::s_GraphicsContext = nullptr;

	GraphicsAPI Graphics::graphicsAPI() {
		return s_GraphicsAPI;
	}

	GraphicsContext& Graphics::graphicsContext() {
#ifdef _DEBUG
		if(s_GraphicsContext == nullptr) throw MILO_RUNTIME_EXCEPTION("GraphicsContext has not been initialized!");
#endif
		return *s_GraphicsContext;
	}

	void Graphics::init() {
		if(graphicsAPI() == GraphicsAPI::Vulkan) {
			s_GraphicsContext = NEW VulkanContext();
		} else {
			// TODO
			throw MILO_RUNTIME_EXCEPTION("Not implemented");
		}
		s_GraphicsContext->init();
	}

	void Graphics::shutdown() {
		DELETE_PTR(s_GraphicsContext);
	}
}
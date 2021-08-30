#include "milo/assets/skybox/SkyboxFactory.h"
#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	SkyboxFactory* SkyboxFactory::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanSkyboxFactory();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
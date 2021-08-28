#include "milo/assets/materials/MaterialResourcePool.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"

namespace milo {

	MaterialResourcePool* MaterialResourcePool::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanMaterialResourcePool();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
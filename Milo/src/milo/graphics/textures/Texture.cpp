#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	Texture2D* Texture2D::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::create();
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Texture2D* Texture2D::createColorAttachment() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::createColorAttachment();
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Texture2D* Texture2D::createDepthAttachment() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::createDepthAttachment();
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

}
#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"

namespace milo {

	Texture2D* Texture2D::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT);
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Texture2D* Texture2D::createColorAttachment() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::create(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Texture2D* Texture2D::createDepthAttachment() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanTexture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Cubemap* Cubemap::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) return VulkanCubemap::create(TEXTURE_USAGE_SAMPLED_BIT);
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
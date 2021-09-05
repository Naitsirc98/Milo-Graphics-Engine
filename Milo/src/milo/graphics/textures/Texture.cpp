#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	Texture2D::Texture2D(TextureUsageFlags usage) {
		m_Id = Assets::textures().nextTextureId();
		m_Usage = usage;
	}

	Texture2D::~Texture2D() {
		Assets::textures().unregisterTexture(*this);
		m_Id = NULL;
	}

	Cubemap::Cubemap(TextureUsageFlags usage) {
		m_Id = Assets::textures().nextTextureId();
		m_Usage = usage;
	}

	Cubemap::~Cubemap() {
		Assets::textures().unregisterTexture(*this);
		m_Id = NULL;
	}

	Texture2D* Texture2D::create(TextureUsageFlags usage) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			auto texture = VulkanTexture2D::create(usage);
			Assets::textures().registerTexture(*texture);
			return texture;
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}

	Cubemap* Cubemap::create(TextureUsageFlags usage) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			auto texture = VulkanCubemap::create(usage);
			Assets::textures().registerTexture(*texture);
			return texture;
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
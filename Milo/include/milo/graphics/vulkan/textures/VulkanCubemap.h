#pragma once

#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/vulkan/textures/VulkanTexture.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"

#define CUBEMAP_ARRAY_LAYERS_COUNT 6

namespace milo {

	class VulkanCubemap : public VulkanTexture, public Cubemap {
		friend class VulkanAllocator;
	public:
		explicit VulkanCubemap(const CreateInfo& createInfo);
		~VulkanCubemap() override;

		uint32_t width() const override;
		uint32_t height() const override;

		void allocate(const Cubemap::AllocInfo& allocInfo) override;
		void update(const Cubemap::UpdateInfo& updateInfo) override;

		void generateMipmaps() override;

	public:
		static VulkanCubemap* create(TextureUsageFlags usage);
	};
}
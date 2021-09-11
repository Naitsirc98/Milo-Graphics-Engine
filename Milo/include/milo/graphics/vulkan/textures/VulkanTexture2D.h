#pragma once

#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "VulkanTexture.h"

namespace milo {

	class VulkanTexture2D : public VulkanTexture, public Texture2D {
		friend class VulkanAllocator;
	private:
		explicit VulkanTexture2D(const CreateInfo& createInfo);
		explicit VulkanTexture2D(const VulkanTexture2D& other) = delete;
	public:
		~VulkanTexture2D() override;

		uint32_t width() const override;
		uint32_t height() const override;

		void allocate(const Texture2D::AllocInfo& allocInfo) override;
		void update(const UpdateInfo& updateInfo) override;

		void generateMipmaps() override;

	protected:
		void doSetName(const String& name) override;
	public:
		static VulkanTexture2D* create(TextureUsageFlags usage);
	};
}
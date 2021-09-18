#pragma once

#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "VulkanTexture.h"

namespace milo {

	class VulkanTexture2D : public VulkanTexture, public Texture2D {
		friend class VulkanAllocator;
	protected:
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
		static VulkanTexture2D* create(const CreateInfo& createInfo);
	};

	class VulkanTexture2DArray : public VulkanTexture2D {
	private:
		ArrayList<VkImageView> m_Layers;
	private:
		explicit VulkanTexture2DArray(const CreateInfo& createInfo);
		explicit VulkanTexture2DArray(const VulkanTexture2D& other) = delete;
	public:
		uint32_t numLayers() const;
		VkImageView getLayer(uint32_t index) const;
		void allocate(const Texture2D::AllocInfo& allocInfo) override;
	protected:
		void destroy() override;
	public:
		static VulkanTexture2DArray* create(TextureUsageFlags usage, uint32_t numLayers);
	};
}
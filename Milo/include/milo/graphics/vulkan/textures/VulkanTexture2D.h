#pragma once

#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanTexture2D : public Texture2D {
		friend class VulkanAllocator;
	public:
		struct CreateInfo {
			VulkanDevice* device;
			VkImageCreateInfo imageInfo = {};
			VkImageViewCreateInfo viewInfo = {};
			VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;

			CreateInfo();
		};
	private:
		VulkanDevice* m_Device;

		VkImage m_VkImage = VK_NULL_HANDLE;
		VkImageCreateInfo m_VkImageInfo = {};

		VkImageView m_VkImageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo m_VkImageViewInfo = {};

		VkSampler m_VkSampler = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaMemoryUsage m_Usage = VMA_MEMORY_USAGE_UNKNOWN;

	public:
		VulkanTexture2D(const CreateInfo& createInfo);
		explicit VulkanTexture2D(const VulkanTexture2D& other) = delete;
		~VulkanTexture2D();

		VulkanTexture2D& operator=(const VulkanTexture2D& other) = delete;

		VulkanDevice* device() const;
		VkImage vkImage() const;
		const VkImageCreateInfo& vkImageInfo() const;
		VkImageView vkImageView() const;
		const VkImageViewCreateInfo& vkImageViewInfo() const;
		VkSampler vkSampler() const;
		void vkSampler(VkSampler sampler);
		VmaAllocation allocation() const;
		VkImageLayout layout() const;
		VmaMemoryUsage usage() const;

		uint32_t width() const override;
		uint32_t height() const override;

		void allocate(const Texture2D::AllocInfo& allocInfo) override;
		void update(const UpdateInfo& updateInfo) override;

		void generateMipmaps() override;

	private:
		void transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& memoryBarrier,
							  VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
		void copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer& buffer);
	public:
		static VulkanTexture2D* create();
		static VulkanTexture2D* createColorAttachment();
		static VulkanTexture2D* createDepthAttachment();
	};
}
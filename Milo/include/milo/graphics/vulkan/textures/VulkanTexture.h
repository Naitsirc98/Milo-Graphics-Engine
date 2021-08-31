#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"

namespace milo {

	class VulkanTexture {
		friend class VulkanAllocator;
	public:

		struct CreateInfo {
			VulkanDevice* device{nullptr};
			VkSampleCountFlagBits samples{VK_SAMPLE_COUNT_1_BIT};
			VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};
			TextureUsageFlags usage{TEXTURE_USAGE_UNDEFINED_BIT};
			uint32_t arrayLayers{1};

			CreateInfo();
		};

		struct TransitionLayoutInfo {
			VkImageMemoryBarrier memoryBarrier{};
			VkPipelineStageFlags srcStage{0};
			VkPipelineStageFlags dstStage{0};
		};
	protected:
		VulkanDevice* m_Device;

		VkImage m_VkImage = VK_NULL_HANDLE;
		VkImageCreateInfo m_ImageInfo = {};

		VkImageView m_VkImageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo m_ViewInfo = {};

		VkSampler m_VkSampler = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VmaMemoryUsage m_Usage = VMA_MEMORY_USAGE_UNKNOWN;

	public:
		explicit VulkanTexture(const CreateInfo& createInfo);
		virtual ~VulkanTexture();

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

		void transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& barrier, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);

		void setLayout(VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);
		void setLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask);

	protected:
		void allocate(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels);
		void copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer& buffer);
	};
}
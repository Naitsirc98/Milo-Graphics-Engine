#pragma once

#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/assets/images/Image.h"

namespace milo {

	struct VulkanTextureAllocInfo {
		VkImageCreateInfo imageInfo = {};
		VkImageViewCreateInfo viewInfo = {};
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VulkanTextureAllocInfo();
	};

	class VulkanTexture {
		friend class VulkanAllocator;
	private:
		VulkanDevice& m_Device;
		ResourceHandle m_Handle = NULL;
		VkImage m_VkImage = VK_NULL_HANDLE;
		VkImageCreateInfo m_VkImageInfo = {};
		VkImageView m_VkImageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo m_VkImageViewInfo = {};
		VkSampler m_VkSampler = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VkImageLayout m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	public:
		explicit VulkanTexture(VulkanDevice& device);
		explicit VulkanTexture(const VulkanTexture& other) = delete;
		~VulkanTexture();

		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] ResourceHandle handle() const;
		[[nodiscard]] VkImage vkImage() const;
		[[nodiscard]] const VkImageCreateInfo& vkImageInfo() const;
		[[nodiscard]] VkImageView vkImageView() const;
		[[nodiscard]] const VkImageViewCreateInfo& vkImageViewInfo() const;
		[[nodiscard]] VkSampler vkSampler() const;
		void vkSampler(VkSampler sampler);
		[[nodiscard]] VmaAllocation allocation() const;
		[[nodiscard]] VkImageLayout layout() const;
		uint32_t width() const;
		uint32_t height() const;
		uint32_t depth() const;

		void allocate(const VulkanTextureAllocInfo& allocInfo);
		void reallocate(const VulkanTextureAllocInfo& allocInfo);
		void destroy();

		void pixels(const Image& image);
		void pixels(uint32_t width, uint32_t height, PixelFormat format, const void* data);

		void generateMipmaps(uint32_t mipLevels = UINT32_MAX);

		VulkanTexture& operator=(const VulkanTexture& other) = delete;

		bool operator==(const VulkanTexture& rhs) const;
		bool operator!=(const VulkanTexture& rhs) const;
	private:
		void transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& memoryBarrier,
							  VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage);
		void copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer* buffer);
	};

	namespace VulkanImageInfos {
		VkImageCreateInfo create() noexcept;
		VkImageCreateInfo colorAttachment() noexcept;
		VkImageCreateInfo depthStencilAttachment() noexcept;
	}

	namespace VulkanImageViewInfos {
		VkImageViewCreateInfo create(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
		VkImageViewCreateInfo colorAttachment(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
		VkImageViewCreateInfo depthStencilAttachment(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
	}

	namespace VulkanSamplerInfos {
		VkSamplerCreateInfo create();
	}
}
#pragma once

#include "milo/graphics/api/vulkan/VulkanAllocator.h"
#include "milo/graphics/api/vulkan/VulkanDevice.h"

namespace milo {

	struct VulkanTextureAllocInfo {
		VkImageCreateInfo imageInfo = {};
		VkImageViewCreateInfo viewInfo = {};
		VkSamplerCreateInfo samplerInfo = {};
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;
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
		VkSamplerCreateInfo m_VkSamplerInfo = {};
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
		[[nodiscard]] const VkSamplerCreateInfo& vkSamplerInfo() const;
		[[nodiscard]] VmaAllocation allocation() const;
		[[nodiscard]] VkImageLayout layout() const;

		void allocate(const VulkanTextureAllocInfo& allocInfo);
		void reallocate(const VulkanTextureAllocInfo& allocInfo);
		void destroy();

		VulkanTexture& operator=(const VulkanTexture& other) = delete;

		bool operator==(const VulkanTexture& rhs) const;
		bool operator!=(const VulkanTexture& rhs) const;
	};
}
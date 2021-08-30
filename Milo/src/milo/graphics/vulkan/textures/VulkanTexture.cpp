#include "milo/graphics/vulkan/textures/VulkanTexture.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanTexture::CreateInfo::CreateInfo() {
		device = VulkanContext::get()->device();
	}

	VulkanTexture::VulkanTexture(const VulkanTexture::CreateInfo& createInfo) : m_Device(createInfo.device) {

		m_ImageInfo = mvk::ImageCreateInfo::create(createInfo.usage);
		m_ImageInfo.arrayLayers = createInfo.arrayLayers;
		m_ImageInfo.tiling = createInfo.tiling;
		m_ImageInfo.samples = createInfo.samples;

		m_ViewInfo = mvk::ImageViewCreateInfo::create();
		m_ViewInfo.subresourceRange.layerCount = createInfo.arrayLayers;

		if((createInfo.usage & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
			m_ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		} else {
			m_ViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		m_VkSampler = VulkanContext::get()->samplerMap()->getDefaultSampler();
	}

	VulkanTexture::~VulkanTexture() {

		VulkanAllocator::get()->freeImage(m_VkImage, m_Allocation);

		VK_CALLV(vkDestroyImageView(m_Device->logical(), m_VkImageView, nullptr));

		m_Allocation = VK_NULL_HANDLE;
		m_VkImage = VK_NULL_HANDLE;
		m_VkImageView = VK_NULL_HANDLE;
		m_VkSampler = VK_NULL_HANDLE;
	}

	VulkanDevice* VulkanTexture::device() const {
		return m_Device;
	}

	VkImage VulkanTexture::vkImage() const {
		return m_VkImage;
	}

	const VkImageCreateInfo& VulkanTexture::vkImageInfo() const {
		return m_ImageInfo;
	}

	VkImageView VulkanTexture::vkImageView() const {
		return m_VkImageView;
	}

	const VkImageViewCreateInfo& VulkanTexture::vkImageViewInfo() const {
		return m_ViewInfo;
	}

	VkSampler VulkanTexture::vkSampler() const {
		return m_VkSampler;
	}

	void VulkanTexture::vkSampler(VkSampler sampler) {
		m_VkSampler = sampler;
	}

	VmaAllocation VulkanTexture::allocation() const {
		return m_Allocation;
	}

	VkImageLayout VulkanTexture::layout() const {
		return m_ImageLayout;
	}

	VmaMemoryUsage VulkanTexture::usage() const {
		return m_Usage;
	}

	void VulkanTexture::transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& barrier,
										 VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {

		VK_CALLV(vkCmdPipelineBarrier(commandBuffer,
									  srcStage, dstStage,
									  0,
									  0, nullptr,
									  0, nullptr,
									  1, &barrier));
	}

	void VulkanTexture::allocate(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels) {

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = m_Usage;
		vmaAllocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		m_ImageInfo.extent = {width, height, 1};
		if(mipLevels == AUTO_MIP_LEVELS) {
			m_ImageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
			m_ViewInfo.subresourceRange.levelCount = m_ImageInfo.mipLevels;
		}

		m_ImageInfo.format = mvk::fromPixelFormat(format);
		m_ViewInfo.format = m_ImageInfo.format;

		VulkanAllocator::get()->allocateImage(m_ImageInfo, vmaAllocInfo, m_VkImage, m_Allocation);

		m_ViewInfo.image = m_VkImage;
		VK_CALL(vkCreateImageView(m_Device->logical(), &m_ViewInfo, nullptr, &m_VkImageView));
	}

	void VulkanTexture::copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer& buffer) {

		VkBufferImageCopy copyRegion{};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = m_ViewInfo.subresourceRange.aspectMask;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = m_ViewInfo.subresourceRange.layerCount;
		copyRegion.imageExtent = m_ImageInfo.extent;

		VK_CALLV(vkCmdCopyBufferToImage(commandBuffer, buffer.vkBuffer(),
										m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion));
	}
}
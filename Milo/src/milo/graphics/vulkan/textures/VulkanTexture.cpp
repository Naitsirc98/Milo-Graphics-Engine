#include "milo/graphics/vulkan/textures/VulkanTexture.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanTexture::CreateInfo::CreateInfo() {
		device = VulkanContext::get()->device();
	}

	VulkanTexture::VulkanTexture(const VulkanTexture::CreateInfo& createInfo) : m_Device(createInfo.device) {
		m_CreateInfo = createInfo;
		create(createInfo);
	}

	void VulkanTexture::create(const VulkanTexture::CreateInfo& createInfo) {

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

		m_VkSampler = m_VkSampler == VK_NULL_HANDLE ? VulkanContext::get()->samplerMap()->getDefaultSampler() : m_VkSampler;
	}

	VulkanTexture::~VulkanTexture() {
		destroy();
	}

	void VulkanTexture::destroy() {

		m_Device->awaitTermination();

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

	VmaMemoryUsage VulkanTexture::memoryUsage() const {
		return m_Usage;
	}

	void VulkanTexture::resize(const Size& size) {

		if(m_VkImage == VK_NULL_HANDLE) return;

		VkImageLayout oldLayout = m_ImageLayout;

		destroy();
		create(m_CreateInfo);
		allocate(size.width, size.height, mvk::toPixelFormat(m_ImageInfo.format), m_ImageInfo.mipLevels);
		if(oldLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
			setLayout(oldLayout);
		}
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

	void VulkanTexture::setLayout(VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		setLayout(m_ImageLayout, newLayout, srcStageMask, dstStageMask);
	}

	void VulkanTexture::setLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask,
								  VkPipelineStageFlags dstStageMask) {
		VulkanTask task{};
		task.run = [&](VkCommandBuffer commandBuffer) { setLayout(commandBuffer, oldLayout, newLayout, srcStageMask, dstStageMask);};
		m_Device->transferCommandPool()->execute(task);
	}

	void VulkanTexture::setLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		setLayout(commandBuffer, m_ImageLayout, newLayout, srcStageMask, dstStageMask);
	}

	void VulkanTexture::setLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout,
								  VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {

		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.image = m_VkImage;
		imageMemoryBarrier.subresourceRange = m_ViewInfo.subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (m_ImageLayout) {
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_GENERAL:
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newLayout) {
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
				commandBuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

		m_ImageLayout = newLayout;
	}

	void VulkanTexture::allocate(uint32_t width, uint32_t height, PixelFormat format, uint32_t mipLevels) {

		if(m_VkImage != VK_NULL_HANDLE) {
			destroy();
		}

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

		m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"

namespace milo {

	VulkanTexture2D::VulkanTexture2D(const VulkanTexture2D::CreateInfo& createInfo)
			: m_Device(createInfo.device), m_VkImageInfo(createInfo.imageInfo),
			  m_VkImageViewInfo(createInfo.viewInfo), m_Usage(createInfo.usage) {

		m_VkImageViewInfo.format = m_VkImageInfo.format;

		m_VkSampler = VulkanSamplerMap::get()->getDefaultSampler();
	}

	VulkanTexture2D::~VulkanTexture2D() {

		VulkanAllocator::get()->freeImage(m_VkImage, m_Allocation);

		VK_CALLV(vkDestroyImageView(m_Device->logical(), m_VkImageView, nullptr));

		m_Allocation = VK_NULL_HANDLE;
		m_VkImage = VK_NULL_HANDLE;
		m_VkImageView = VK_NULL_HANDLE;
		m_VkSampler = VK_NULL_HANDLE;
	}

	VulkanDevice* VulkanTexture2D::device() const {
		return m_Device;
	}

	VkImage VulkanTexture2D::vkImage() const {
		return m_VkImage;
	}

	const VkImageCreateInfo& VulkanTexture2D::vkImageInfo() const {
		return m_VkImageInfo;
	}

	VkImageView VulkanTexture2D::vkImageView() const {
		return m_VkImageView;
	}

	const VkImageViewCreateInfo& VulkanTexture2D::vkImageViewInfo() const {
		return m_VkImageViewInfo;
	}

	VkSampler VulkanTexture2D::vkSampler() const {
		return m_VkSampler;
	}

	void VulkanTexture2D::vkSampler(VkSampler sampler) {
		m_VkSampler = sampler;
	}

	VmaAllocation VulkanTexture2D::allocation() const {
		return m_Allocation;
	}

	VkImageLayout VulkanTexture2D::layout() const {
		return m_ImageLayout;
	}

	VmaMemoryUsage VulkanTexture2D::usage() const {
		return m_Usage;
	}

	uint32_t VulkanTexture2D::width() const {
		return m_VkImageInfo.extent.width;
	}

	uint32_t VulkanTexture2D::height() const {
		return m_VkImageInfo.extent.height;
	}

	void VulkanTexture2D::allocate(const AllocInfo& allocInfo) {

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = m_Usage;
		vmaAllocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;


		m_VkImageInfo.extent = {allocInfo.width, allocInfo.height, 1};
		if(allocInfo.mipLevels == AUTO_MIP_LEVELS) {
			m_VkImageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(allocInfo.width, allocInfo.height)))) + 1;
		}

		m_VkImageInfo.format = mvk::fromPixelFormat(allocInfo.format);
		m_VkImageViewInfo.format = m_VkImageInfo.format;

		VulkanAllocator::get()->allocateImage(m_VkImageInfo, vmaAllocInfo, m_VkImage, m_Allocation);

		if(allocInfo.pixels != nullptr) {
			UpdateInfo updateInfo = {};
			updateInfo.size = allocInfo.width * allocInfo.height * PixelFormats::size(allocInfo.format);
			updateInfo.pixels = allocInfo.pixels;
			update(updateInfo);
		}

		m_VkImageViewInfo.image = m_VkImage;
		VK_CALL(vkCreateImageView(m_Device->logical(), &m_VkImageViewInfo, nullptr, &m_VkImageView));
	}

	void VulkanTexture2D::update(const UpdateInfo& updateInfo) {

		VulkanBuffer* stagingBuffer = VulkanBuffer::createStagingBuffer(updateInfo.pixels, updateInfo.size);

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_VkImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = m_VkImageInfo.mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			barrier.oldLayout = m_ImageLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			transitionLayout(commandBuffer, barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			copyFromBuffer(commandBuffer, *stagingBuffer);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = 0;

			transitionLayout(commandBuffer, barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			m_ImageLayout = barrier.newLayout;
		};

		m_Device->transferCommandPool()->execute(task);

		DELETE_PTR(stagingBuffer);
	}

	void VulkanTexture2D::generateMipmaps() {

		if(m_VkImage == VK_NULL_HANDLE) return;
		if(m_VkImageInfo.mipLevels == 1) return;

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties = {};
		VK_CALLV(vkGetPhysicalDeviceFormatProperties(m_Device->physical(), m_VkImageInfo.format, &formatProperties));

		if((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
			throw MILO_RUNTIME_EXCEPTION("Failed to generate mipmaps: TextureAsset image format does not support linear blitting");
		}

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			uint32_t mipLevels = m_VkImageInfo.mipLevels;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_VkImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = m_VkImageInfo.mipLevels;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			// Transition all levels to DST OPTIMAL
			if(m_ImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				barrier.oldLayout = m_ImageLayout;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				transitionLayout(commandBuffer, barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			}

			barrier.subresourceRange.levelCount = 1; // 1 Mip level at a time

			int32_t mipWidth = static_cast<int32_t>(width());
			int32_t mipHeight = static_cast<int32_t>(height());

			for (uint32_t i = 1; i < mipLevels; ++i) {

				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				VK_CALLV(vkCmdPipelineBarrier(commandBuffer,
											  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
											  0, nullptr,
											  0, nullptr,
											  1, &barrier));

				VkImageBlit blit = {};
				blit.srcOffsets[0] = {0, 0, 0};
				blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;
				blit.dstOffsets[0] = {0, 0, 0};
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				VK_CALLV(vkCmdBlitImage(commandBuffer,
							   m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							   m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   1, &blit,
							   VK_FILTER_LINEAR));

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				VK_CALLV(vkCmdPipelineBarrier(commandBuffer,
									 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
									 0, nullptr,
									 0, nullptr,
									 1, &barrier));

				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			VK_CALLV(vkCmdPipelineBarrier(commandBuffer,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
								 0, nullptr,
								 0, nullptr,
								 1, &barrier));

			m_ImageLayout = barrier.newLayout;
		};

		m_Device->graphicsCommandPool()->execute(task);

	}

	void VulkanTexture2D::transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& memoryBarrier,
										   VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {

		VK_CALLV(vkCmdPipelineBarrier(commandBuffer,
							 srcStage, dstStage,
							 0,
							 0, nullptr,
							 0, nullptr,
							 1, &memoryBarrier));
	}

	void VulkanTexture2D::copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer& buffer) {

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = m_VkImageInfo.extent;

		VK_CALLV(vkCmdCopyBufferToImage(commandBuffer, buffer.vkBuffer(), m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion));
	}

	VulkanTexture2D* VulkanTexture2D::create() {
		return new VulkanTexture2D(CreateInfo());
	}

	VulkanTexture2D* VulkanTexture2D::create(PixelFormat pixelFormat, TextureUsageFlags usageFlags) {

		VulkanTexture2D::CreateInfo createInfo = {};

		createInfo.imageInfo.format = mvk::fromPixelFormat(pixelFormat);

		if((usageFlags & TEXTURE_USAGE_SAMPLED_BIT) != 0) {
			createInfo.imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			createInfo.viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if((usageFlags & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) != 0) {
			createInfo.imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			createInfo.viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		if((usageFlags & TEXTURE_USAGE_DEPTH_ATTACHMENT_BIT) != 0 || (usageFlags & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
			createInfo.imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			createInfo.viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}

		return new VulkanTexture2D(createInfo);
	}

	VulkanTexture2D* VulkanTexture2D::createColorAttachment() {

		CreateInfo createInfo = {};
		createInfo.imageInfo = mvk::ImageCreateInfo::colorAttachment();
		createInfo.viewInfo = mvk::ImageViewCreateInfo::colorAttachment();

		VkFormat format = VulkanContext::get()->swapchain()->format();

		createInfo.imageInfo.format = format;
		createInfo.viewInfo.format = format;

		return new VulkanTexture2D(createInfo);
	}

	VulkanTexture2D* VulkanTexture2D::createDepthAttachment() {

		CreateInfo createInfo = {};
		createInfo.imageInfo = mvk::ImageCreateInfo::depthStencilAttachment();
		createInfo.viewInfo = mvk::ImageViewCreateInfo::depthStencilAttachment();

		VkFormat depthFormat = createInfo.device->depthFormat();

		createInfo.imageInfo.format = depthFormat;
		createInfo.viewInfo.format = depthFormat;

		return new VulkanTexture2D(createInfo);
	}

	VulkanTexture2D::CreateInfo::CreateInfo() {
		device = VulkanContext::get()->device();
		imageInfo = mvk::ImageCreateInfo::create();
		viewInfo = mvk::ImageViewCreateInfo::create();
	}
}
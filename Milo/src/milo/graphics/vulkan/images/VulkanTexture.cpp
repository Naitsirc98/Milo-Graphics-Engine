#include "milo/graphics/vulkan/images/VulkanTexture.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	static Atomic<ResourceHandle> g_TextureHandleProvider = 1; // TODO: graphics factory

	VulkanTexture::VulkanTexture(VulkanDevice& device) : m_Device(device) {
		m_Handle = g_TextureHandleProvider++;
	}

	VulkanTexture::~VulkanTexture() {
		destroy();
		m_Handle = NULL;
	}

	VulkanDevice& VulkanTexture::device() const {
		return m_Device;
	}

	ResourceHandle VulkanTexture::handle() const {
		return m_Handle;
	}

	VkImage VulkanTexture::vkImage() const {
		return m_VkImage;
	}

	const VkImageCreateInfo& VulkanTexture::vkImageInfo() const {
		return m_VkImageInfo;
	}

	VkImageView VulkanTexture::vkImageView() const {
		return m_VkImageView;
	}

	const VkImageViewCreateInfo& VulkanTexture::vkImageViewInfo() const {
		return m_VkImageViewInfo;
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

	uint32_t VulkanTexture::width() const {
		return m_VkImageInfo.extent.width;
	}

	uint32_t VulkanTexture::height() const {
		return m_VkImageInfo.extent.height;
	}

	uint32_t VulkanTexture::depth() const {
		return m_VkImageInfo.extent.depth;
	}

	void VulkanTexture::allocate(const VulkanTextureAllocInfo& allocInfo) {
		m_Device.context().allocator().allocateImage(*this, allocInfo.imageInfo, allocInfo.usage);

		m_VkImageViewInfo = allocInfo.viewInfo;
		m_VkImageViewInfo.image = m_VkImage;
		m_VkImageViewInfo.format = m_VkImageInfo.format;

		VK_CALL(vkCreateImageView(m_Device.ldevice(), &m_VkImageViewInfo, nullptr, &m_VkImageView));
	}

	void VulkanTexture::reallocate(const VulkanTextureAllocInfo& allocInfo) {
		destroy();
		allocate(allocInfo);
	}

	void VulkanTexture::destroy() {
		m_Device.context().allocator().freeImage(*this);

		VK_CALLV(vkDestroyImageView(m_Device.ldevice(), m_VkImageView, nullptr));

		m_VkImageView = VK_NULL_HANDLE;
		m_VkSampler = VK_NULL_HANDLE;

		m_VkImageInfo = {};
		m_VkImageViewInfo = {};
	}

	void VulkanTexture::pixels(const Image& image) {
		pixels(image.width(), image.height(), image.format(), image.pixels());
	}

	// TODO: This is a specific for fragment shader readable textures
	void VulkanTexture::pixels(uint32_t width, uint32_t height, PixelFormat format, const void* data) {

		VulkanBuffer* stagingBuffer = VulkanBuffer::createStagingBuffer(data, width * height * PixelFormats::size(format));

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			VkImageMemoryBarrier barrierToTransferDst = mvk::vkImageMemoryBarrier(m_VkImage, m_ImageLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			barrierToTransferDst.srcAccessMask = 0;
			barrierToTransferDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			transitionLayout(commandBuffer, barrierToTransferDst, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			copyFromBuffer(commandBuffer, stagingBuffer);

			VkImageMemoryBarrier barrierToShaderRead = mvk::vkImageMemoryBarrier(m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			barrierToShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrierToShaderRead.dstAccessMask = 0;
			transitionLayout(commandBuffer, barrierToShaderRead, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		};

		m_Device.transferCommandPool()->execute(task);

		m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		DELETE_PTR(stagingBuffer);
	}

	void VulkanTexture::generateMipmaps(uint32_t mipLevels) {

		if(m_VkImage == VK_NULL_HANDLE) return;

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties = {};
		vkGetPhysicalDeviceFormatProperties(m_Device.pdevice(), m_VkImageInfo.format, &formatProperties);

		if((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
			throw MILO_RUNTIME_EXCEPTION("Failed to generate mipmaps: Texture image format does not support linear blitting");
		}

		if(mipLevels == UINT32_MAX) {
			mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width(), height())))) + 1;
		} else if(mipLevels % 2 != 0) {
			throw MILO_RUNTIME_EXCEPTION("MipLevels must be multiple of 2");
		}

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			uint32_t mipLevels = m_VkImageInfo.mipLevels;

			VkImageMemoryBarrier barrier = mvk::vkImageMemoryBarrier(m_VkImage, m_ImageLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

			int32_t mipWidth = width();
			int32_t mipHeight = height();

			for(uint32_t i = 1;i < mipLevels;++i) {

				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = m_ImageLayout;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
									 0, 0, nullptr, 0, nullptr, 1, &barrier);

				VkImageBlit blit = {};

				blit.srcOffsets[0] = {0,0,0};
				blit.srcOffsets[1] = {mipWidth, mipHeight, 1};

				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;

				blit.dstOffsets[0] = {0, 0, 0};
				blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};

				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;

				vkCmdBlitImage(commandBuffer,
							   vkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
							   vkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
							   1, &blit, VK_FILTER_LINEAR);

				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = m_ImageLayout;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
									 0, 0, nullptr, 0, nullptr, 1, &barrier);

				if(mipWidth > 1) mipWidth /= 2;
				if(mipHeight > 1) mipHeight /= 2;
			}

			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
								 0, 0, nullptr, 0, nullptr, 1, &barrier);
		};

		m_Device.transferCommandPool()->execute(task);
	}

	void VulkanTexture::transitionLayout(VkCommandBuffer commandBuffer, const VkImageMemoryBarrier& memoryBarrier,
										 VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {

		vkCmdPipelineBarrier(commandBuffer,
							 srcStage, dstStage,
							 0,
							 0, nullptr,
							 0, nullptr,
							 1, &memoryBarrier);
	}

	void VulkanTexture::copyFromBuffer(VkCommandBuffer commandBuffer, VulkanBuffer* buffer) {

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = m_VkImageInfo.extent;

		vkCmdCopyBufferToImage(commandBuffer, buffer->vkBuffer(), m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
	}

	bool VulkanTexture::operator==(const VulkanTexture& rhs) const {
		return m_Handle == rhs.m_Handle;
	}

	bool VulkanTexture::operator!=(const VulkanTexture& rhs) const {
		return ! (rhs == *this);
	}

	VulkanTextureAllocInfo::VulkanTextureAllocInfo() {
		imageInfo = VulkanImageInfos::create();
		viewInfo = VulkanImageViewInfos::create();
	}


	// ======


	VkImageCreateInfo VulkanImageInfos::create() noexcept {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.arrayLayers = 1;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.mipLevels = 1;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		return imageInfo;
	}

	VkImageCreateInfo VulkanImageInfos::colorAttachment() noexcept {
		VkImageCreateInfo imageInfo = create();
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		return imageInfo;
	}

	VkImageCreateInfo VulkanImageInfos::depthStencilAttachment() noexcept {
		VkImageCreateInfo imageInfo = create();
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		return imageInfo;
	}


	// ====


	VkImageViewCreateInfo VulkanImageViewInfos::create(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.image = image;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = levelCount;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		return viewInfo;
	}

	VkImageViewCreateInfo VulkanImageViewInfos::colorAttachment(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
		return create(image, format, levelCount);
	}

	VkImageViewCreateInfo VulkanImageViewInfos::depthStencilAttachment(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
		VkImageViewCreateInfo viewInfo = create(image, format, levelCount);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		return viewInfo;
	}


	//  ====


	VkSamplerCreateInfo VulkanSamplerInfos::create() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.mipLodBias = 0;
		return samplerInfo;
	}
}
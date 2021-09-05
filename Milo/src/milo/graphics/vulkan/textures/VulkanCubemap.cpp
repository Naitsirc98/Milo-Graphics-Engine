#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanCubemap::VulkanCubemap(const VulkanTexture::CreateInfo& createInfo) : VulkanTexture(createInfo), Cubemap(createInfo.usage) {

		if(createInfo.arrayLayers != 6) throw MILO_RUNTIME_EXCEPTION("Cubemaps must have 6 array layers");

		m_ImageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		m_ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}

	VulkanCubemap::~VulkanCubemap() = default;

	uint32_t VulkanCubemap::width() const {
		return m_ImageInfo.extent.width;
	}

	uint32_t VulkanCubemap::height() const {
		return m_ImageInfo.extent.height;
	}

	void VulkanCubemap::allocate(const Cubemap::AllocInfo& allocInfo) {

		VulkanTexture::allocate(allocInfo.width, allocInfo.height, allocInfo.format, allocInfo.mipLevels);

		if(allocInfo.pixels != nullptr) {
			UpdateInfo updateInfo = {};
			updateInfo.size = allocInfo.width * allocInfo.height * PixelFormats::size(allocInfo.format);
			updateInfo.pixels = allocInfo.pixels;
			update(updateInfo);
		}
	}

	void VulkanCubemap::update(const Cubemap::UpdateInfo& updateInfo) {

		VulkanBuffer* stagingBuffer = VulkanBuffer::createStagingBuffer(updateInfo.pixels, updateInfo.size);

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_VkImage;
			barrier.subresourceRange = m_ViewInfo.subresourceRange;

			barrier.oldLayout = m_ImageLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			transitionLayout(commandBuffer, barrier, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VulkanTexture::copyFromBuffer(commandBuffer, *stagingBuffer);

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

	void VulkanCubemap::generateMipmaps() {

		if(m_VkImage == VK_NULL_HANDLE) return;
		if(m_ImageInfo.mipLevels == 1) return;

		// Check if image format supports linear blitting
		VkFormatProperties formatProperties = {};
		VK_CALLV(vkGetPhysicalDeviceFormatProperties(m_Device->physical(), m_ImageInfo.format, &formatProperties));

		if((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
			throw MILO_RUNTIME_EXCEPTION("Failed to generate mipmaps: TextureAsset image format does not support linear blitting");
		}

		VulkanTask task = {};
		task.run = [&](VkCommandBuffer commandBuffer) {

			uint32_t mipLevels = m_ImageInfo.mipLevels;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_VkImage;
			barrier.subresourceRange = m_ViewInfo.subresourceRange;

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
				blit.srcSubresource.layerCount = m_ViewInfo.subresourceRange.layerCount;
				blit.dstOffsets[0] = {0, 0, 0};
				blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = m_ViewInfo.subresourceRange.layerCount;

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

	VulkanCubemap* VulkanCubemap::create(TextureUsageFlags usage) {

		CreateInfo createInfo{};
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.arrayLayers = CUBEMAP_ARRAY_LAYERS_COUNT;
		createInfo.usage = usage;

		return new VulkanCubemap(createInfo);
	}
}
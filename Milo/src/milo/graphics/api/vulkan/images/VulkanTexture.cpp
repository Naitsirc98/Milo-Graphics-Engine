#include "milo/graphics/api/vulkan/images/VulkanTexture.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"

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

	const VkSamplerCreateInfo& VulkanTexture::vkSamplerInfo() const {
		return m_VkSamplerInfo;
	}

	VmaAllocation VulkanTexture::allocation() const {
		return m_Allocation;
	}

	VkImageLayout VulkanTexture::layout() const {
		return m_ImageLayout;
	}

	void VulkanTexture::allocate(const VulkanTextureAllocInfo& allocInfo) {
		m_Device.context().allocator().allocateImage(*this, allocInfo.imageInfo, allocInfo.usage);


		VK_CALL(vkCreateImageView(m_Device.ldevice(), &allocInfo.viewInfo, nullptr, &m_VkImageView));
		VK_CALL(vkCreateSampler(m_Device.ldevice(), &allocInfo.samplerInfo, nullptr, &m_VkSampler));

		m_VkImageViewInfo = allocInfo.viewInfo;
		m_VkImageViewInfo.image = m_VkImage;
		m_VkSamplerInfo = allocInfo.samplerInfo;
	}

	void VulkanTexture::reallocate(const VulkanTextureAllocInfo& allocInfo) {
		destroy();
		allocate(allocInfo);
	}

	void VulkanTexture::destroy() {
		m_Device.context().allocator().freeImage(*this);

		vkDestroyImageView(m_Device.ldevice(), m_VkImageView, nullptr);
		vkDestroySampler(m_Device.ldevice(), m_VkSampler, nullptr);

		m_VkImageView = VK_NULL_HANDLE;
		m_VkSampler = VK_NULL_HANDLE;

		m_VkImageInfo = {};
		m_VkImageViewInfo = {};
		m_VkSamplerInfo = {};
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
		samplerInfo = VulkanSamplerInfos::create();
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
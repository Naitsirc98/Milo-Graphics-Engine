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

	const VkImage_T* VulkanTexture::vkImage() const {
		return m_VkImage;
	}

	const VkImageCreateInfo& VulkanTexture::vkImageInfo() const {
		return m_VkImageInfo;
	}

	const VkImageView_T* VulkanTexture::vkImageView() const {
		return m_VkImageView;
	}

	const VkImageViewCreateInfo& VulkanTexture::vkImageViewInfo() const {
		return m_VkImageViewInfo;
	}

	const VkSampler_T* VulkanTexture::vkSampler() const {
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
}
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"

namespace milo {

	VulkanFramebuffer::VulkanFramebuffer(VulkanDevice& device, const VkFramebufferCreateInfo& info) : m_Device(device),
																									  m_Info(info) {

		VK_CALL(vkCreateFramebuffer(device.ldevice(), &m_Info, nullptr, &m_VkFramebuffer));
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		vkDestroyFramebuffer(m_Device.ldevice(), m_VkFramebuffer, nullptr);
		m_VkFramebuffer = VK_NULL_HANDLE;
	}

	VulkanDevice& VulkanFramebuffer::device() const {
		return m_Device;
	}

	VkFramebuffer VulkanFramebuffer::vkFramebuffer() const {
		return m_VkFramebuffer;
	}

	const VkFramebufferCreateInfo& VulkanFramebuffer::info() const {
		return m_Info;
	}

}
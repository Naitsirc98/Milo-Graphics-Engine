#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanFramebuffer {
	private:
		VulkanDevice& m_Device;
		VkFramebuffer m_VkFramebuffer = VK_NULL_HANDLE;
		VkFramebufferCreateInfo m_Info = {};
	public:
		VulkanFramebuffer(VulkanDevice& device, const VkFramebufferCreateInfo& info);
		~VulkanFramebuffer();
		VulkanDevice& device() const;
		VkFramebuffer vkFramebuffer() const;
		const VkFramebufferCreateInfo& info() const;
	};
}
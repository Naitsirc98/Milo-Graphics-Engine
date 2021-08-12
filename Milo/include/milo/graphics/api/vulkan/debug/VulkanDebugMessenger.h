#pragma once

#include "milo/graphics/api/vulkan/VulkanContext.h"

namespace milo {

	class VulkanDebugMessenger {
	private:
		VulkanContext* m_Context;
		VkDebugUtilsMessengerEXT m_VkDebugMessenger = VK_NULL_HANDLE;
	public:
		explicit VulkanDebugMessenger(VulkanContext* context);
		~VulkanDebugMessenger();
		[[nodiscard]] VkDebugUtilsMessengerEXT vkDebugUtilsMessenger() const;
		static VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();
	};
}
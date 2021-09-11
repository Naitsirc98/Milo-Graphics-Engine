#pragma once

#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	class VulkanDebugMessenger {
	private:
		VulkanContext* m_Context;
		VkDebugUtilsMessengerEXT m_VkDebugMessenger = VK_NULL_HANDLE;
	public:
		explicit VulkanDebugMessenger(VulkanContext* context);
		~VulkanDebugMessenger();
		 VkDebugUtilsMessengerEXT vkDebugUtilsMessenger() const;
	public:
		static void setName(VkImage image, const char* name);
		static void setName(VkImageView imageView, const char* name);
		static void setName(VkBuffer buffer, const char* name);
		static VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo();
	};
}
#pragma once

#define GLFW_INCLUDE_VULKAN
#include "milo/graphics/Window.h"
#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;

	class VulkanWindowSurface {
	private:
		VulkanContext& m_Context;
		Window* m_Window;
		VkSurfaceKHR m_VkSurface = VK_NULL_HANDLE;
	public:
		VulkanWindowSurface(VulkanContext& context, Window* window);
		~VulkanWindowSurface();
		[[nodiscard]] Window* window() const;
		[[nodiscard]] VkSurfaceKHR vkSurface() const;
	};
}

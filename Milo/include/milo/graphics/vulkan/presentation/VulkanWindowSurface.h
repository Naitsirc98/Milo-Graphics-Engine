#pragma once

#define GLFW_INCLUDE_VULKAN
#include "milo/graphics/Window.h"
#include "milo/graphics/vulkan/VulkanAPI.h"

namespace milo {

	class VulkanContext;

	class VulkanWindowSurface {
	private:
		VulkanContext* m_Context;
		Window* m_Window;
		VkSurfaceKHR m_VkSurface = VK_NULL_HANDLE;
	public:
		VulkanWindowSurface(VulkanContext* context, Window* window);
		~VulkanWindowSurface();
		VulkanContext* context() const;
		Window* window() const;
		VkSurfaceKHR vkSurface() const;
	};

	struct VulkanWindowSurfaceDetails {
		VkPhysicalDevice physicalDevice;
		VulkanWindowSurface* surface;

		VulkanWindowSurfaceDetails(VkPhysicalDevice d, VulkanWindowSurface* s);
		VkSurfaceCapabilitiesKHR capabilities() const;
		ArrayList<VkSurfaceFormatKHR> formats() const;
		ArrayList<VkPresentModeKHR> presentModes() const;
		VkSurfaceFormatKHR getBestSurfaceFormat() const;
		VkPresentModeKHR getDefaultPresentMode() const;
		uint32_t getSwapchainImageCount() const;
		VkExtent2D extent() const;
	};
}

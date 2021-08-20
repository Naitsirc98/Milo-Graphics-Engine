#pragma once

#define GLFW_INCLUDE_VULKAN
#include "milo/graphics/Window.h"
#include "milo/graphics/vulkan/VulkanAPI.h"

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

	struct VulkanWindowSurfaceDetails {
		VkPhysicalDevice physicalDevice;
		VulkanWindowSurface& surface;

		VulkanWindowSurfaceDetails(VkPhysicalDevice d, VulkanWindowSurface& s);
		[[nodiscard]] VkSurfaceCapabilitiesKHR capabilities() const;
		[[nodiscard]] ArrayList<VkSurfaceFormatKHR> formats() const;
		[[nodiscard]] ArrayList<VkPresentModeKHR> presentModes() const;
		[[nodiscard]] VkSurfaceFormatKHR getBestSurfaceFormat() const;
		[[nodiscard]] VkPresentModeKHR getDefaultPresentMode() const;
		[[nodiscard]] uint32_t getSwapchainImageCount() const;
		[[nodiscard]] VkExtent2D extent() const;
	};
}

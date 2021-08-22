#include <algorithm>
#include "milo/graphics/vulkan/presentation/VulkanWindowSurface.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanWindowSurface::VulkanWindowSurface(VulkanContext* context, Window* window) : m_Context(context), m_Window(window) {
		VK_CALL(glfwCreateWindowSurface(context->vkInstance(), window->handle(), nullptr, &m_VkSurface));
	}

	VulkanWindowSurface::~VulkanWindowSurface() {
		vkDestroySurfaceKHR(m_Context->vkInstance(), m_VkSurface, nullptr);
		m_VkSurface = VK_NULL_HANDLE;
		m_Window = nullptr;
	}

	VulkanContext* VulkanWindowSurface::context() const {
		return m_Context;
	}

	Window* VulkanWindowSurface::window() const {
		return m_Window;
	}

	VkSurfaceKHR VulkanWindowSurface::vkSurface() const {
		return m_VkSurface;
	}

	// ======


	VulkanWindowSurfaceDetails::VulkanWindowSurfaceDetails(VkPhysicalDevice d, VulkanWindowSurface* s) : physicalDevice(d), surface(s) {
	}

	inline VkSurfaceCapabilitiesKHR VulkanWindowSurfaceDetails::capabilities() const {
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface->vkSurface(), &capabilities);
		return capabilities;
	}

	inline ArrayList<VkSurfaceFormatKHR> VulkanWindowSurfaceDetails::formats() const {
		uint32_t formatsCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->vkSurface(), &formatsCount, nullptr);
		ArrayList<VkSurfaceFormatKHR> formats(formatsCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface->vkSurface(), &formatsCount, formats.data());
		return formats;
	}

	inline ArrayList<VkPresentModeKHR> VulkanWindowSurfaceDetails::presentModes() const {
		uint32_t presentModesCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->vkSurface(), &presentModesCount, nullptr);
		ArrayList<VkPresentModeKHR> presentModes(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface->vkSurface(), &presentModesCount, presentModes.data());
		return presentModes;
	}

	VkSurfaceFormatKHR VulkanWindowSurfaceDetails::getBestSurfaceFormat() const {
		ArrayList<VkSurfaceFormatKHR> formats = this->formats();

		uint32_t chosen = 0;
		for(uint32_t i = 0;i < formats.size();++i) {
			const VkSurfaceFormatKHR& format = formats[i];
			if(format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				chosen = i;
				break;
			}
		}
		return formats[chosen];
	}

	VkPresentModeKHR VulkanWindowSurfaceDetails::getDefaultPresentMode() const {
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanWindowSurfaceDetails::extent() const {
		VkSurfaceCapabilitiesKHR capabilities = this->capabilities();

		if(capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX) {
			return capabilities.currentExtent; // Optimal
		}

		Size fbSize = surface->window()->size();
		VkExtent2D actualExtent = {};
		actualExtent.width = fbSize.width;
		actualExtent.height = fbSize.height;

		VkExtent2D minExtent = capabilities.minImageExtent;
		VkExtent2D maxExtent = capabilities.maxImageExtent;

		actualExtent.width = std::clamp(actualExtent.width, minExtent.width, maxExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, minExtent.height, maxExtent.height);

		return actualExtent;
	}

	uint32_t VulkanWindowSurfaceDetails::getSwapchainImageCount() const {
		const VkSurfaceCapabilitiesKHR capabilities = this->capabilities();
		return std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
	}
}

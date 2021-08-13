#include "milo/graphics/api/vulkan/VulkanWindowSurface.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"

namespace milo {

	VulkanWindowSurface::VulkanWindowSurface(VulkanContext& context, Window *window) : m_Context(context), m_Window(window) {
		VK_CALL(glfwCreateWindowSurface(context.vkInstance(), window->handle(), nullptr, &m_VkSurface));
	}

	VulkanWindowSurface::~VulkanWindowSurface() {
		vkDestroySurfaceKHR(m_Context.vkInstance(), m_VkSurface, nullptr);
		m_VkSurface = VK_NULL_HANDLE;
		m_Window = nullptr;
	}

	Window *VulkanWindowSurface::window() const {
		return m_Window;
	}

	VkSurfaceKHR VulkanWindowSurface::vkSurface() const {
		return m_VkSurface;
	}
}

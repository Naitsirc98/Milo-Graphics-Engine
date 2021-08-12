#include "milo/graphics/api/vulkan/VulkanExtensionsAndLayers.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace milo {

	ArrayList<const char *> VulkanExtensions::getInstanceExtensions() {
		uint32_t count;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

		ArrayList<const char*> extensions;
		extensions.reserve(count + 1);
		for(uint32_t i = 0;i < count;++i) extensions.push_back(glfwExtensions[i]);

#ifdef _DEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

return extensions;
	}

	ArrayList<const char *> VulkanExtensions::getDeviceExtensions(DeviceUsageFlags usageFlags) {
		if((usageFlags & DeviceUsagePresentationBit) != 0) return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
		return {};
	}

	ArrayList<const char *> VulkanLayers::getInstanceLayers() {
#ifdef _DEBUG
		return {"VK_LAYER_KHRONOS_validation"};
#else
		return {};
#endif
	}

	ArrayList<const char *> VulkanLayers::getDeviceLayers(DeviceUsageFlags usageFlags) {
#ifdef _DEBUG
		return {"VK_LAYER_KHRONOS_validation"};
#else
		return {};
#endif
	}
}

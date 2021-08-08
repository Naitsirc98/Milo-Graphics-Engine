#include "milo/Milo.h"
#include <vulkan/vulkan.h>
#include <iostream>

uint32_t getPhysicalDeviceCount()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Milo";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	const char* layerNames = "VK_LAYER_KHRONOS_validation";
	const char* extensions = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = &layerNames;
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = &extensions;

	VkInstance instance;

	VkResult r = vkCreateInstance(&createInfo, nullptr, &instance);

	std::cout << r << std::endl;

	uint32_t count;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);

	return count;
}

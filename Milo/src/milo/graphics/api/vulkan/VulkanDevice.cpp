#include "milo/graphics/api/vulkan/VulkanDevice.h"

namespace milo {

	VulkanDevice::VulkanDevice(const VulkanContext& context) : m_Context(context) {
	}

	VulkanDevice::~VulkanDevice() {
		waitFor();

		m_GraphicsQueue = VK_NULL_HANDLE;
		m_ComputeQueue = VK_NULL_HANDLE;
		m_TransferQueue = VK_NULL_HANDLE;
		m_PresentationQueue = VK_NULL_HANDLE;

		vkDestroyDevice(m_Ldevice, nullptr);
		m_Ldevice = VK_NULL_HANDLE;
		m_Pdevice = VK_NULL_HANDLE;
	}

	void VulkanDevice::init(const VulkanDevice::Info& info) {

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.enabledExtensionCount = info.extensionNames.size();
		createInfo.ppEnabledExtensionNames = info.extensionNames.data();
		createInfo.enabledLayerCount = info.layerNames.size();
		createInfo.ppEnabledLayerNames = info.layerNames.data();

		// TODO
	}

	void VulkanDevice::waitFor() {
		vkDeviceWaitIdle(m_Ldevice);
	}

	void VulkanDevice::waitFor(VkQueue queue) {
		vkQueueWaitIdle(queue);
	}

	VulkanContext& VulkanDevice::context() const {
		return m_Info.context;
	}

	VkPhysicalDevice VulkanDevice::pdevice() const {
		return m_Pdevice;
	}

	VkDevice VulkanDevice::ldevice() const {
		return m_Ldevice;
	}

	VkQueue VulkanDevice::graphicsQueue() const {
		return m_GraphicsQueue;
	}

	VkQueue VulkanDevice::computeQueue() const {
		return m_ComputeQueue;
	}

	VkQueue VulkanDevice::transferQueue() const {
		return m_TransferQueue;
	}

	VkQueue VulkanDevice::presentationQueue() const {
		return m_PresentationQueue;
	}

	VulkanPhysicalDeviceInfo VulkanDevice::pDeviceInfo() const {
		return {m_Pdevice};
	}

	ArrayList<VkPhysicalDevice> VulkanDevice::listAllPhysicalDevices(VkInstance vkInstance) {
		uint32_t count;
		vkEnumeratePhysicalDevices(vkInstance, &count, nullptr);
		ArrayList<VkPhysicalDevice> pdevices(count);
		vkEnumeratePhysicalDevices(vkInstance, &count, pdevices.data());
		return pdevices;
	}

	VulkanPhysicalDeviceInfo::VulkanPhysicalDeviceInfo(const VkPhysicalDevice physicalDevice)
		: physicalDevice(physicalDevice) {}

	VkPhysicalDeviceProperties VulkanDevice::properties() const {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(m_Pdevice, &deviceProperties);
		return deviceProperties;
	}

	VkPhysicalDeviceFeatures VulkanDevice::features() const {
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(m_Pdevice, &deviceFeatures);
		return deviceFeatures;
	}

	VkPhysicalDeviceMemoryProperties VulkanDevice::memoryProperties() const {
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(m_Pdevice, &memoryProperties);
		return memoryProperties;
	}

	ArrayList<VkQueueFamilyProperties> VulkanDevice::queueFamilyProperties() const {
		uint32_t count;
		vkGetPhysicalDeviceQueueFamilyProperties(m_Pdevice, &count, nullptr);
		ArrayList<VkQueueFamilyProperties> queueFamilyProperties(count);
		vkGetPhysicalDeviceQueueFamilyProperties(m_Pdevice, &count, queueFamilyProperties.data());
		return queueFamilyProperties;
	}

	ArrayList<VkExtensionProperties> VulkanPhysicalDeviceInfo::extensions() const {
		uint32_t count;
		vkEnumerateDeviceExtensionProperties(physicalDevice, &count, nullptr);
		ArrayList<VkExtensionProperties> extensionProperties(count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, &count, extensionProperties.data());
		return extensionProperties;
	}

	ArrayList<VkLayerProperties> VulkanPhysicalDeviceInfo::layers() const {
		uint32_t count;
		vkEnumerateDeviceLayerProperties(physicalDevice, &count, nullptr);
		ArrayList<VkLayerProperties> layerProperties(count);
		vkEnumerateDeviceLayerProperties(physicalDevice, &count, layerProperties.data());
		return layerProperties;
	}
}
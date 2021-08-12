#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/api/vulkan/VulkanExtensionsAndLayers.h"
#include "milo/graphics/api/vulkan/debug/VulkanDebugMessenger.h"

namespace milo {

	VulkanContext::VulkanContext() = default;

	VulkanContext::~VulkanContext() {
		DELETE_PTR(m_Device);

		DELETE_PTR(m_DebugMessenger);

		vkDestroyInstance(m_VkInstance, nullptr);
		m_VkInstance = VK_NULL_HANDLE;
	}

	Handle VulkanContext::handle() const {
		return vkInstance();
	}

	VkInstance VulkanContext::vkInstance() const {
#ifdef _DEBUG
		if(m_VkInstance == VK_NULL_HANDLE) throw MILO_RUNTIME_EXCEPTION("VkInstance has not been initialized!");
#endif
		return m_VkInstance;
	}

	VulkanDevice& VulkanContext::device() const {
#ifdef _DEBUG
		if(m_Device == nullptr) throw MILO_RUNTIME_EXCEPTION("Device has not been initialized!");
#endif
		return *m_Device;
	}

	void VulkanContext::init() {
		Log::info("Initializing Vulkan Context...");
		createVkInstance();
		createDebugMessenger();
		createMainVulkanDevice();
		Log::info("Vulkan Context initialized");
	}

	void VulkanContext::createVkInstance() {

		VkApplicationInfo applicationInfo = getApplicationInfo();
		ArrayList<const char*> extensions = VulkanExtensions::getInstanceExtensions();
		ArrayList<const char*> layers = VulkanLayers::getInstanceLayers();

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &applicationInfo;
		createInfo.enabledExtensionCount = extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = layers.size();
		createInfo.ppEnabledLayerNames = layers.data();

#ifdef _DEBUG
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = VulkanDebugMessenger::getDebugMessengerCreateInfo();
		createInfo.pNext = &debugMessengerCreateInfo;
#endif

		VK_CALL(vkCreateInstance(&createInfo, nullptr, &m_VkInstance));
	}

	void VulkanContext::createDebugMessenger() {
		m_DebugMessenger = new VulkanDebugMessenger(this);
	}

	void VulkanContext::createMainVulkanDevice() {
		ArrayList<RankedDevice> physicalDeviceRank = VulkanDevice::rankAllPhysicalDevices(m_VkInstance);
		if(physicalDeviceRank.empty()) {
			throw MILO_RUNTIME_EXCEPTION("Failed to find a Vulkan supported device");
		}

		const RankedDevice& bestDevice = physicalDeviceRank.front();

		VulkanDevice::Info deviceInfo = {};
		deviceInfo.physicalDevice = bestDevice.physicalDevice;
		deviceInfo.usageFlags = DeviceUsageAllBit;
		deviceInfo.extensionNames = VulkanExtensions::getDeviceExtensions(deviceInfo.usageFlags);
		deviceInfo.layerNames = VulkanLayers::getDeviceLayers(deviceInfo.usageFlags);

		m_Device = NEW VulkanDevice(*this);
		m_Device->init(deviceInfo);

		Log::info(m_Device->name() + " chosen as the preferred GPU with a score of " + str(bestDevice.score));
	}

	VkApplicationInfo VulkanContext::getApplicationInfo() {
		VkApplicationInfo applicationInfo = {};
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.apiVersion = VK_VERSION_1_2;
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		applicationInfo.pApplicationName = "Milo";
		applicationInfo.pEngineName = "Milo Graphics Engine";
		return applicationInfo;
	}
}
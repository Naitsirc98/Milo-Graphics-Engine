#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/VulkanExtensionsAndLayers.h"
#include "milo/graphics/vulkan/debug/VulkanDebugMessenger.h"

namespace milo {

	VulkanContext::VulkanContext() = default;

	VulkanContext::~VulkanContext() {

		m_Device->awaitTermination();

		DELETE_PTR(m_Presenter);
		DELETE_PTR(m_Allocator);
		DELETE_PTR(m_Swapchain);
		DELETE_PTR(m_Device);
		DELETE_PTR(m_WindowSurface);
		DELETE_PTR(m_DebugMessenger);

		vkDestroyInstance(m_VkInstance, nullptr);
		m_VkInstance = VK_NULL_HANDLE;
	}

	Handle VulkanContext::handle() const {
		return vkInstance();
	}

	VkInstance VulkanContext::vkInstance() const {
		return m_VkInstance;
	}

	VulkanDevice& VulkanContext::device() const {
		return *m_Device;
	}

	VulkanWindowSurface &VulkanContext::windowSurface() const {
		return *m_WindowSurface;
	}

	VulkanSwapchain& VulkanContext::swapchain() const {
		return *m_Swapchain;
	}

	VulkanAllocator& VulkanContext::allocator() const {
		return *m_Allocator;
	}

	GraphicsPresenter& VulkanContext::presenter() const {
		return *m_Presenter;
	}

	VulkanPresenter& VulkanContext::vulkanPresenter() const {
		return *m_Presenter;
	}

	void VulkanContext::init(Window& mainWindow) {
		Log::info("Initializing Vulkan Context...");
		{
			createVkInstance();
			createDebugMessenger();
			createWindowSurface(mainWindow);
			createMainVulkanDevice();
			createSwapchain();
			createAllocator();
			createPresenter();
		}
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

	void VulkanContext::createWindowSurface(Window& mainWindow) {
		m_WindowSurface = new VulkanWindowSurface(*this, &mainWindow);
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

		m_Device = new VulkanDevice(*this);
		m_Device->init(deviceInfo);

		Log::info(m_Device->name() + " chosen as the preferred GPU with a score of " + str(bestDevice.score));
	}

	void VulkanContext::createSwapchain() {
		m_Swapchain = new VulkanSwapchain(*this);
	}

	void VulkanContext::createAllocator() {
		m_Allocator = new VulkanAllocator(*this);
	}

	void VulkanContext::createPresenter() {
		m_Presenter = new VulkanPresenter(*this);
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
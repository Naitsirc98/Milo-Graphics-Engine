#include <milo/events/EventSystem.h>
#include "milo/graphics/api/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"

namespace milo {

	VulkanSwapchain::VulkanSwapchain(VulkanContext& context) : m_Context(context) {

		create();

		Size lastWindowSize = Window::get().size();

		EventSystem::addEventCallback(EventType::WindowResize, [&](const Event& e) {
			const Size& size = Window::get().size();//static_cast<const WindowResizeEvent&>(e).size;
			if(size == lastWindowSize || size.aspect() == 0.0f) return;
			recreate();
			lastWindowSize = size;
		});
	}

	VulkanSwapchain::~VulkanSwapchain() {
		destroy();
	}

	VulkanContext& VulkanSwapchain::context() const {
		return m_Context;
	}

	const VkSwapchainKHR_T* VulkanSwapchain::vkSwapchain() const {
		return m_VkSwapchain;
	}

	VkPresentModeKHR VulkanSwapchain::presentMode() const {
		return m_PresentMode;
	}

	VkFormat VulkanSwapchain::format() const {
		return m_Format;
	}

	const VkExtent2D& VulkanSwapchain::extent() const {
		return m_Extent;
	}

	const VkImage* VulkanSwapchain::images() const {
		return m_Images;
	}

	uint32_t VulkanSwapchain::imageCount() const {
		return m_ImageCount;
	}

	void VulkanSwapchain::addSwapchainResetCallback(SwapchainResetCallback resetCallback) {
		m_OnResetCallbacks.push_back(std::move(resetCallback));
	}

	void VulkanSwapchain::create() {
		createSwapchain();
		getSwapchainImages();
	}

	void VulkanSwapchain::createSwapchain() {

		VulkanWindowSurfaceDetails surfaceDetails(m_Context.device().pdevice(), m_Context.windowSurface());

		VkSurfaceCapabilitiesKHR capabilities = surfaceDetails.capabilities();
		VkSurfaceFormatKHR format = surfaceDetails.getBestSurfaceFormat();
		if(m_PresentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) m_PresentMode = surfaceDetails.getDefaultPresentMode();
		VkExtent2D extent = surfaceDetails.extent();
		uint32_t imageCount = surfaceDetails.getSwapchainImageCount();

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Context.windowSurface().vkSurface();

		// Image settings
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = format.format;
		createInfo.imageColorSpace = format.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		const VulkanDeviceQueue& graphicsQueue = m_Context.device().graphicsQueue();
		const VulkanDeviceQueue& presentationQueue = m_Context.device().presentationQueue();

		const uint32_t queueFamilies[2] = {presentationQueue.family, graphicsQueue.family};

		if(graphicsQueue != presentationQueue) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 1;
		}

		createInfo.pQueueFamilyIndices = queueFamilies;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_PresentMode;
		createInfo.clipped = true;

		createInfo.oldSwapchain = m_VkSwapchain;

		VK_CALL(vkCreateSwapchainKHR(m_Context.device().ldevice(), &createInfo, nullptr, &m_VkSwapchain));

		m_Format = format.format;
		m_Extent = extent;
		m_ImageCount = imageCount;
	}

	void VulkanSwapchain::destroy() {
		DELETE_ARRAY(m_Images);

		m_OnResetCallbacks.clear();

		vkDestroySwapchainKHR(m_Context.device().ldevice(), m_VkSwapchain, nullptr);
		m_VkSwapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::recreate() {
#ifdef _DEBUG
		Size size = Window::get().size();
		Log::debug("Recreating Swapchain (Size = {} x {})", size.width, size.height);
#endif
		destroy();
		create();
		for(auto& callback : m_OnResetCallbacks) callback();
	}

	void VulkanSwapchain::getSwapchainImages() {
		if(m_Images != nullptr) DELETE_ARRAY(m_Images);
		m_Images = new VkImage[m_ImageCount];
		vkGetSwapchainImagesKHR(m_Context.device().ldevice(), m_VkSwapchain, &m_ImageCount, m_Images);
	}
}
#include <milo/events/EventSystem.h>
#include "milo/graphics/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanSwapchain::VulkanSwapchain(VulkanContext& context) : m_Context(context) {

		create();

		Size lastWindowSize = Window::get().size();

		EventSystem::addEventCallback(EventType::WindowResize, [&](const Event& e) {
			const Size& size = Window::get().size();
			if(size == lastWindowSize || size.aspect() == 0.0f) return;
			recreate();
			lastWindowSize = size;
		});
	}

	VulkanSwapchain::~VulkanSwapchain() {
		destroy();
		m_OnRecreateCallbacks.clear();
	}

	VulkanContext& VulkanSwapchain::context() const {
		return m_Context;
	}

	VulkanDevice& VulkanSwapchain::device() const {
		return m_Context.device();
	}

	VkSwapchainKHR VulkanSwapchain::vkSwapchain() const {
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

	const VulkanSwapchainImage* VulkanSwapchain::images() const {
		return m_Images;
	}

	uint32_t VulkanSwapchain::imageCount() const {
		return m_ImageCount;
	}

	void VulkanSwapchain::addSwapchainRecreateCallback(SwapchainResetCallback callback) {
		m_OnRecreateCallbacks.push_back(std::move(callback));
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

		const VulkanQueue& graphicsQueue = m_Context.device().graphicsQueue();
		const VulkanQueue& presentationQueue = m_Context.device().presentationQueue();

		const uint32_t queueFamilies[2] = {presentationQueue.family, graphicsQueue.family};

		if(graphicsQueue != presentationQueue) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
		} else {
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

		device().awaitTermination();

		for(uint32_t i = 0;i < m_ImageCount;++i) {
			destroySwapchainImage(m_Images[i]);
		}

		vkDestroySwapchainKHR(m_Context.device().ldevice(), m_VkSwapchain, nullptr);
		m_VkSwapchain = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::recreate() {
#ifdef _DEBUG
		Size size = Window::get().size();
		Log::debug("Recreating Swapchain (Size = {} x {})", size.width, size.height);
		float startMillis = Time::millis();
#endif
		destroy();
		create();
		for(auto& callback : m_OnRecreateCallbacks) callback();
#ifdef _DEBUG
		Log::debug("Swapchain (Size = {} x {}) recreated after {} ms", size.width, size.height, Time::millis() - startMillis);
#endif
	}

	void VulkanSwapchain::getSwapchainImages() {
		VkImage images[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		vkGetSwapchainImagesKHR(m_Context.device().ldevice(), m_VkSwapchain, &m_ImageCount, images);

		for(uint32_t i = 0;i < m_ImageCount;++i) {
			createSwapchainImage(i, m_Images[i], images[i]);
		}
	}

	void VulkanSwapchain::createSwapchainImage(uint32_t index, VulkanSwapchainImage& image, VkImage vkImage) {
		image.index = index;
		image.vkImage = vkImage;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vkImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_Format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VK_CALL(vkCreateImageView(m_Context.device().ldevice(), &viewInfo, nullptr, &image.vkImageView));
		image.vkImageViewInfo = viewInfo;
	}

	void VulkanSwapchain::destroySwapchainImage(VulkanSwapchainImage& image) {
		vkDestroyImageView(m_Context.device().ldevice(), image.vkImageView, nullptr);
		image.vkImageView = VK_NULL_HANDLE;
		image.vkImage = VK_NULL_HANDLE;
		image.index = 0;
	}
}
#pragma once

#include "VulkanWindowSurface.h"
#include "milo/graphics/api/vulkan/images/VulkanTexture.h"

namespace milo {

	using SwapchainResetCallback = Function<void>;

	struct VulkanSwapchainImage {
		uint32_t index = 0;
		VkImage vkImage = VK_NULL_HANDLE;
		VkImageView vkImageView = VK_NULL_HANDLE;
		VkImageViewCreateInfo vkImageViewInfo = {};
	};

	class VulkanSwapchain {
		friend class VulkanContext;
	private:
		VulkanContext& m_Context;
		VkSwapchainKHR m_VkSwapchain = nullptr;
		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkFormat m_Format = VK_FORMAT_MAX_ENUM;
		VkExtent2D m_Extent = {0, 0};
		VulkanSwapchainImage m_Images[MAX_SWAPCHAIN_IMAGE_COUNT]{};
		uint32_t m_ImageCount = 0;
		ArrayList<SwapchainResetCallback> m_OnRecreateCallbacks;
	public:
		explicit VulkanSwapchain(VulkanContext& context);
		~VulkanSwapchain();
		[[nodiscard]] VulkanContext& context() const;
		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] VkSwapchainKHR vkSwapchain() const;
		[[nodiscard]] VkPresentModeKHR presentMode() const;
		[[nodiscard]] VkFormat format() const;
		[[nodiscard]] const VkExtent2D& extent() const;
		[[nodiscard]] const VulkanSwapchainImage* images() const;
		[[nodiscard]] uint32_t imageCount() const;
		void addSwapchainRecreateCallback(SwapchainResetCallback callback);
		void recreate();
	private:
		void create();
		void createSwapchain();
		void destroy();
		void getSwapchainImages();
		void destroySwapchainImage(VulkanSwapchainImage& image);
		void createSwapchainImage(uint32_t index, VulkanSwapchainImage& image, VkImage vkImage);
	};
}
#pragma once

#include "VulkanWindowSurface.h"
#include "milo/graphics/api/vulkan/images/VulkanImage.h"

namespace milo {

	using SwapchainResetCallback = Function<void>;

	class VulkanSwapchain {
		friend class VulkanContext;
	private:
		VulkanContext& m_Context;
		VkSwapchainKHR m_VkSwapchain = nullptr;
		VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkFormat m_Format = VK_FORMAT_MAX_ENUM;
		VkExtent2D m_Extent = {0, 0};
		VkImage* m_Images = nullptr;
		uint32_t m_ImageCount = 0;
		ArrayList<SwapchainResetCallback> m_OnResetCallbacks;
	public:
		explicit VulkanSwapchain(VulkanContext& context);
		~VulkanSwapchain();
		[[nodiscard]] VulkanContext& context() const;
		[[nodiscard]] const VkSwapchainKHR_T* vkSwapchain() const;
		[[nodiscard]] VkPresentModeKHR presentMode() const;
		[[nodiscard]] VkFormat format() const;
		[[nodiscard]] const VkExtent2D& extent() const;
		[[nodiscard]] const VkImage* images() const;
		[[nodiscard]] uint32_t imageCount() const;
		void addSwapchainResetCallback(SwapchainResetCallback resetCallback);
	private:
		void create();
		void createSwapchain();
		void destroy();
		void recreate();
		void getSwapchainImages();
	};
}
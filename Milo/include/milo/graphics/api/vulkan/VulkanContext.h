#pragma once

#include "milo/graphics/api/GraphicsContext.h"
#include "milo/graphics/api/vulkan/VulkanAPI.h"
#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/api/vulkan/VulkanAllocator.h"
#include "milo/graphics/api/vulkan/presentation/VulkanPresenter.h"

namespace milo {

	class VulkanDebugMessenger;

	class VulkanContext : public GraphicsContext {
		friend class Graphics;
	private:
		VkInstance m_VkInstance = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
		VulkanWindowSurface* m_WindowSurface = nullptr;
		VulkanDebugMessenger* m_DebugMessenger = nullptr;
		VulkanSwapchain* m_Swapchain = nullptr;
		VulkanAllocator* m_Allocator = nullptr;
		VulkanPresenter* m_Presenter = nullptr;
	private:
		VulkanContext();
		~VulkanContext() override;
	public:
		[[nodiscard]] Handle handle() const override;
		[[nodiscard]] VkInstance vkInstance() const;
		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] VulkanWindowSurface& windowSurface() const;
		[[nodiscard]] VulkanSwapchain& swapchain() const;
		[[nodiscard]] VulkanAllocator& allocator() const;
		[[nodiscard]] GraphicsPresenter& presenter() const override;
		[[nodiscard]] VulkanPresenter& vulkanPresenter() const;
	protected:
		void init(Window& mainWindow) override;
	private:
		void createVkInstance();
		void createDebugMessenger();
		void createWindowSurface(Window& mainWindow);
		void createMainVulkanDevice();
		void createSwapchain();
		void createAllocator();
		void createPresenter();
	private:
		static VkApplicationInfo getApplicationInfo();

	};
}
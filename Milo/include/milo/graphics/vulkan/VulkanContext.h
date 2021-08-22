#pragma once

#include "milo/graphics/GraphicsContext.h"
#include "VulkanAPI.h"
#include "VulkanDevice.h"
#include "milo/graphics/vulkan/presentation/VulkanSwapchain.h"
#include "VulkanAllocator.h"
#include "milo/graphics/vulkan/presentation/VulkanPresenter.h"
#include "milo/common/Common.h"

namespace milo {

	class VulkanDebugMessenger;

	class VulkanContext : public GraphicsContext {
		friend class Graphics;
	private:
		VkInstance m_VkInstance = VK_NULL_HANDLE;
		VulkanDevice* m_Device;
		VulkanWindowSurface* m_WindowSurface;
		VulkanDebugMessenger* m_DebugMessenger;
		VulkanSwapchain* m_Swapchain;
		VulkanAllocator* m_Allocator;
		VulkanPresenter* m_Presenter;
	private:
		VulkanContext();
	public:
		~VulkanContext() override;
		Handle handle() const override;
		VkInstance vkInstance() const;
		VulkanDevice* device() const;
		VulkanWindowSurface* windowSurface() const;
		VulkanSwapchain* swapchain() const;
		VulkanAllocator* allocator() const;
		GraphicsPresenter* presenter() const override;
		VulkanPresenter* vulkanPresenter() const;
	protected:
		void init(Window* mainWindow) override;
	private:
		void createVkInstance();
		void createDebugMessenger();
		void createWindowSurface(Window* mainWindow);
		void createMainVulkanDevice();
		void createSwapchain();
		void createAllocator();
		void createPresenter();
	private:
		static VulkanContext* s_Instance;
	public:
		static VulkanContext* get();
	private:
		static VkApplicationInfo getApplicationInfo();
	};
}
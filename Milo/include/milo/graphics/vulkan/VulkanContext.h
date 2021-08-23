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
		VulkanDevice* m_Device = nullptr;
		VulkanWindowSurface* m_WindowSurface = nullptr;
		VulkanDebugMessenger* m_DebugMessenger = nullptr;
		VulkanSwapchain* m_Swapchain = nullptr;
		VulkanAllocator* m_Allocator = nullptr;
		VulkanPresenter* m_Presenter = nullptr;
		VulkanSamplerMap* m_SamplerMap = nullptr;
	private:
		VulkanContext();
		~VulkanContext() override;
	public:
		Handle handle() const override;
		VkInstance vkInstance() const;
		VulkanDevice* device() const;
		VulkanWindowSurface* windowSurface() const;
		VulkanSwapchain* swapchain() const;
		VulkanAllocator* allocator() const;
		GraphicsPresenter* presenter() const override;
		VulkanPresenter* vulkanPresenter() const;
		VulkanSamplerMap* samplerMap() const;
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
		void createSamplerMap();
	private:
		static VulkanContext* s_Instance;
	public:
		static VulkanContext* get();
	private:
		static VkApplicationInfo getApplicationInfo();
	};
}
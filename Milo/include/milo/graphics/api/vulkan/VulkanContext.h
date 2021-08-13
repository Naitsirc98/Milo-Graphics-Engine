#pragma once

#include "VulkanAPI.h"

#include "milo/graphics/api/GraphicsContext.h"
#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/VulkanWindowSurface.h"

namespace milo {

	class VulkanDebugMessenger;

	class VulkanContext : public GraphicsContext {
		friend class Graphics;
	private:
		VkInstance m_VkInstance = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
		VulkanWindowSurface* m_WindowSurface = nullptr;
		VulkanDebugMessenger* m_DebugMessenger = nullptr;
	private:
		VulkanContext();
		~VulkanContext() override;
	public:
		[[nodiscard]] Handle handle() const override;
		[[nodiscard]] VkInstance vkInstance() const;
		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] const VulkanWindowSurface& windowSurface() const;
	protected:
		void init(Window& mainWindow) override;
	private:
		void createVkInstance();
		void createDebugMessenger();
		void createWindowSurface(Window& mainWindow);
		void createMainVulkanDevice();
	private:
		static VkApplicationInfo getApplicationInfo();
	};

}
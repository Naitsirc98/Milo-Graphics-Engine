#pragma once

#include "VulkanAPI.h"

#include "milo/graphics/api/GraphicsContext.h"
#include "milo/graphics/api/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanContext : public GraphicsContext {
		friend class Graphics;
	private:
		VkInstance m_VkInstance = VK_NULL_HANDLE;
		VulkanDevice* m_Device = nullptr;
	private:
		VulkanContext();
		~VulkanContext() override;
	public:
		[[nodiscard]] Handle handle() const override;
		[[nodiscard]] VkInstance vkInstance() const;
		[[nodiscard]] VulkanDevice& device() const;
	protected:
		void init() override;
	private:
		void createVkInstance();
		void createMainVulkanDevice();
	private:
		static VkApplicationInfo getApplicationInfo();
	};

}
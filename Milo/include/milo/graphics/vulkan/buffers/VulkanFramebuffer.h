#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/rendering/Framebuffer.h"

namespace milo {

	class VulkanFramebuffer : public Framebuffer {
	public:
		struct ApiInfo {
			VulkanDevice* device;
			VkRenderPass renderPass;
		};
	private:
		VulkanDevice* m_Device;
		VkRenderPass m_RenderPass{VK_NULL_HANDLE};
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	public:
		VulkanFramebuffer(const CreateInfo& createInfo);
		~VulkanFramebuffer();
		VulkanDevice* device() const;
		VkRenderPass renderPass() const;
		VkFramebuffer vkFramebuffer() const;
		void resize(const Size& size) override;
	private:
		void destroy();
		void create();
	};
}
#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/rendering/Framebuffer.h"

namespace milo {

	class VulkanFramebuffer : public Framebuffer {
	public:
		struct ApiInfo {
			VulkanDevice* device;
		};
	private:
		VulkanDevice* m_Device;
		mutable HashMap<VkRenderPass, VkFramebuffer> m_VkFramebuffers;
	public:
		VulkanFramebuffer(const CreateInfo& createInfo);
		VulkanFramebuffer(const VulkanFramebuffer& other) = delete;
		~VulkanFramebuffer();
		VulkanDevice* device() const;
		bool hasFramebuffer(VkRenderPass renderPass) const;
		VkFramebuffer get(VkRenderPass renderPass);
		void resize(const Size& size) override;
		VulkanFramebuffer& operator=(const VulkanFramebuffer& other) = delete;
	private:
		void destroyAllFramebuffers();
		void destroyFramebuffer(VkFramebuffer framebuffer);
		VkFramebuffer createFramebuffer(VkRenderPass renderPass);
	};
}
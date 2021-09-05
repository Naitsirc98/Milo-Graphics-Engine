#pragma once

#include "milo/editor/UIRenderer.h"
#include "milo/graphics/vulkan/VulkanAPI.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"

namespace milo {

	class VulkanUIRenderer : public UIRenderer {
		friend class MiloEditor;
	private:
		VkRenderPass m_RenderPass{VK_NULL_HANDLE};
		Array<VulkanTexture2D*, MAX_SWAPCHAIN_IMAGE_COUNT> m_DepthBuffers{};
		Array<VkFramebuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_Framebuffers{};

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_PrimaryCommandBuffers{};
		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_SecondaryCommandBuffers{};
	private:
		VulkanUIRenderer();
		~VulkanUIRenderer();
	public:
		void begin() override;
		void end() override;
	private:
		void initUIBackend();
		void createRenderPass();
		void createDepthBuffers();
		void createFramebuffers();

		void shutdownUIBackend() const;

	private:
		static VulkanDescriptorPool* s_DescriptorPool;
	public:
		static VkDescriptorSet allocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo);
	};
}
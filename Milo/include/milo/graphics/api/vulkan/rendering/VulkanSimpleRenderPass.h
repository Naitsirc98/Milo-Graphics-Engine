#pragma once

#include "milo/graphics/api/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/api/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/api/vulkan/images/VulkanTexture.h"
#include "milo/graphics/api/vulkan/buffers/VulkanBuffer.h"

namespace milo {

	class VulkanSimpleRenderPass {
	private:
		VulkanSwapchain& m_Swapchain;
		VkRenderPass m_VkRenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_VkPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_VkGraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineCache m_VkPipelineCache = VK_NULL_HANDLE;
		VulkanCommandPool* m_CommandPool = nullptr;
		VkCommandBuffer m_VkCommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VkFramebuffer m_VkFramebuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VulkanTexture* m_DepthTextures[MAX_SWAPCHAIN_IMAGE_COUNT]{nullptr};
		VulkanBuffer* m_VertexBuffer = nullptr;
	public:
		explicit VulkanSimpleRenderPass(VulkanSwapchain& swapchain);
		~VulkanSimpleRenderPass();
		void execute(uint32_t swapchainImageIndex);
	private:
		void create();
		void destroy();
		void recreate();
		void updatePushConstants(VkCommandBuffer commandBuffer, const Matrix4& mvp);
		void createRenderPass();
		void createDepthTextures();
		void createFramebuffers();
		void createPipelineLayout();
		void createGraphicsPipeline();
		void createCommandPool();
		void allocateCommandBuffers();
		void createVertexBuffer();
	};
}
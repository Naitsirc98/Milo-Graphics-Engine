#pragma once

#include "milo/graphics/api/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/api/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/api/vulkan/images/VulkanTexture.h"
#include "milo/graphics/api/vulkan/buffers/VulkanBuffer.h"

namespace milo {

	class VulkanSimpleRenderPass {
	public:
		struct ExecuteInfo {
			uint32_t swapchainImageIndex = UINT32_MAX;
			VkSemaphore* waitSemaphores = nullptr;
			uint32_t waitSemaphoresCount = 0;
			VkSemaphore* signalSemaphores = nullptr;
			uint32_t signalSemaphoresCount = 0;
			VkFence fence = VK_NULL_HANDLE;
			VkPipelineStageFlags* waitDstStageMask = nullptr;
		};
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
		void execute(const ExecuteInfo& input);
		void recreate();
	private:
		void create();
		void destroy();
		void createRenderAreaDependentComponents();
		void destroyRenderAreaDependentComponents();
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
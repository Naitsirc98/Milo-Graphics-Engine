#pragma once

#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanFinalRenderPass : public FinalRenderPass {
		friend class FinalRenderPass;
	private:
		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		VkFramebuffer m_Framebuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};

		VkDescriptorSetLayout m_TextureDescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_TextureDescriptorPool{nullptr};

		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_GraphicsPipeline{VK_NULL_HANDLE};

		VulkanCommandPool* m_CommandPool{nullptr};
		VkCommandBuffer m_CommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
	public:
		VulkanFinalRenderPass();
		~VulkanFinalRenderPass();
		void compile(FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();

		void createFramebuffers();

		void createTextureDescriptorLayout();
		void createTextureDescriptorPool();
		void createTextureDescriptorSets(FrameGraphResourcePool* resourcePool);

		void createPipelineLayout();
		void createGraphicsPipeline();

		void createCommandPool();
		void createCommandBuffers();
	};

}
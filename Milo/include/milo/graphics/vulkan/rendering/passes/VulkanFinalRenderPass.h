#pragma once

#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/presentation/VulkanSwapchain.h"

namespace milo {

	class VulkanFinalRenderPass : public FinalRenderPass {
		friend class FinalRenderPass;
	private:
		VulkanDevice* m_Device{nullptr};
		VulkanSwapchain* m_Swapchain{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		Array<VkFramebuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_Framebuffers{};

		VkDescriptorSetLayout m_TextureDescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_TextureDescriptorPool{nullptr};

		VulkanGraphicsPipeline* m_GraphicsPipeline{nullptr};

		VulkanCommandPool* m_CommandPool{nullptr};
		VkCommandBuffer m_CommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Size m_LastSwapchainSize{};
		Size m_LastFramebufferSize{};

	public:
		VulkanFinalRenderPass();
		~VulkanFinalRenderPass();
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();

		void createFramebuffers(FrameGraphResourcePool* pool);

		void createTextureDescriptorLayout();
		void createTextureDescriptorPool();
		void createTextureDescriptorSets(FrameGraphResourcePool* resourcePool);

		void createGraphicsPipeline();

		void createCommandPool();
		void createCommandBuffers(FrameGraphResourcePool* resourcePool);

		void createSemaphores();

		void destroyTransientResources();
	};

}
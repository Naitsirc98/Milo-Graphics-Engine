#pragma once

#include "milo/graphics/rendering/passes/DepthRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"

namespace milo {

	class VulkanDepthRenderPass : public DepthRenderPass {
		friend class DepthRenderPass;
	private:
		struct UniformBuffer {
			Matrix4 cascadeShadowMapMatrices[4];
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VulkanUniformBuffer<UniformBuffer>* m_UniformBuffer{nullptr};

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Size m_LastFramebufferSize{};

	private:
		VulkanDepthRenderPass();
		~VulkanDepthRenderPass();
	public:
		bool shouldCompile(Scene* scene) const;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool);
		void execute(Scene* scene);
	private:
		void buildCommandBuffers(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene, Entity cameraEntity);
		void createRenderPass();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createUniformBuffer();
		void createDescriptorSets();
		void createGraphicsPipeline();
		void createSemaphores();
	};
}
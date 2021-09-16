#pragma once

#include "milo/graphics/rendering/passes/ShadowMapRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	class VulkanShadowMapRenderPass : public ShadowMapRenderPass {
		friend class ShadowMapRenderPass;
	private:
		struct ShadowData {
			Matrix4 viewProjectionMatrix[MAX_SHADOW_CASCADES];
		};

		struct PushConstants {
			Matrix4 modelMatrix;
			uint32_t cascadeIndex;
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VulkanUniformBuffer<ShadowData>* m_UniformBuffer{nullptr};

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Array<Ref<VulkanFramebuffer>, MAX_SWAPCHAIN_IMAGE_COUNT> m_Framebuffers{};

		Size m_LastFramebufferSize{};

	private:
		VulkanShadowMapRenderPass();
		~VulkanShadowMapRenderPass();
	public:
		bool shouldCompile(Scene* scene) const;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool);
		void execute(Scene* scene);
	private:
		void buildCommandBuffers(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void createRenderPass();
		void createDescriptorSetLayoutAndPool();
		void createUniformBuffer();
		void createDescriptorSets();
		void createGraphicsPipeline();
		void createSemaphores();
		void createFramebuffers(const Size& size, FrameGraphResourcePool* resourcePool);
	};
}
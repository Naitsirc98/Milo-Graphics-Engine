#pragma once

#include "milo/graphics/rendering/passes/SkyboxRenderPass.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"

namespace milo {

	class VulkanSkyboxRenderPass : public SkyboxRenderPass {
	public:
		struct UniformBuffer {
			Matrix4 inverseProjViewMatrix;
			float textureLOD;
			float intensity;
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		Array<VkFramebuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_Framebuffers{};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VulkanUniformBuffer<UniformBuffer>* m_UniformBuffer{nullptr};

		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_GraphicsPipeline{VK_NULL_HANDLE};

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

	public:
		VulkanSkyboxRenderPass();
		~VulkanSkyboxRenderPass() override;
		void compile(FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void updateDescriptorSets(Scene* scene);
		void createRenderPass();
		void createFramebuffers(FrameGraphResourcePool* resourcePool);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createUniformBuffer();
		void createDescriptorSets();
		void createPipelineLayout();
		void createGraphicsPipeline();
		void createCommandBuffers();
	};

}
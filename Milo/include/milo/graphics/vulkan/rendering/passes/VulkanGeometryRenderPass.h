#pragma once

#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"

namespace milo {

	class VulkanGeometryRenderPass : public GeometryRenderPass {
		friend class GeometryRenderPass;
	private:
		struct MaterialUniformBufferData {

		};

		struct PushConstants {

		};
	private:
		VulkanDevice* m_Device = nullptr;

		VkFramebuffer m_Framebuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanBuffer* m_CameraUniformBuffer = nullptr;
		VkDescriptorSetLayout m_CameraDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_CameraDescriptorPool = nullptr;

		VulkanBuffer* m_MaterialUniformBuffer = nullptr;
		VkDescriptorSetLayout m_MaterialDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_MaterialDescriptorPool = nullptr;

		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_GraphicsPipeline = VK_NULL_HANDLE;

		VulkanCommandPool* m_CommandPool = nullptr;
		VkCommandBuffer m_CommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
	public:
		VulkanGeometryRenderPass();
		~VulkanGeometryRenderPass();
		void compile(FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();
		void createFramebuffers(FrameGraphResourcePool* resourcePool);

		void createCameraUniformBuffer();
		void createCameraDescriptorPool();
		void createCameraDescriptorSets();

		void createMaterialUniformBuffer();
		void createMaterialDescriptorPool();
		void createMaterialDescriptorSets();

		void createPipelineLayout();
		void createGraphicsPipeline();

		void createCommandPool();
		void createCommandBuffers();
	};

}

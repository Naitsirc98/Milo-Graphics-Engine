#pragma once

#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	class VulkanGeometryRenderPass : public GeometryRenderPass {
		friend class GeometryRenderPass;
	private:
		struct CameraData {
			Matrix4 proj;
			Matrix4 view;
			Matrix4 projView;
		};

		struct PushConstants {
			Matrix4 modelMatrix;
		};
	private:
		VulkanDevice* m_Device = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanUniformBuffer<CameraData>* m_CameraUniformBuffer = nullptr;
		VkDescriptorSetLayout m_CameraDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_CameraDescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		VulkanCommandPool* m_CommandPool = nullptr;
		VkCommandBuffer m_CommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

	public:
		VulkanGeometryRenderPass();
		~VulkanGeometryRenderPass();
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();

		void createCameraUniformBuffer();
		void createCameraDescriptorLayout();
		void createCameraDescriptorPool();
		void createCameraDescriptorSets();

		void createGraphicsPipeline();

		void createCommandPool();
		void createCommandBuffers();

		void createSemaphores();

		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
	};

}

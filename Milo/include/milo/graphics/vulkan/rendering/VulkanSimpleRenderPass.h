#pragma once

#include "milo/graphics/vulkan/presentation/VulkanSwapchain.h"
#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanSamplerMap.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"

#define MAX_DESCRIPTOR_SETS 1024 * 1024

namespace milo {

	class VulkanSimpleRenderPass {
	public:
		struct ExecuteInfo {
			uint32_t currentFrame = UINT32_MAX;
			uint32_t swapchainImageIndex = UINT32_MAX;
			VkSemaphore* waitSemaphores = nullptr;
			uint32_t waitSemaphoresCount = 0;
			VkSemaphore* signalSemaphores = nullptr;
			uint32_t signalSemaphoresCount = 0;
			VkFence fence = VK_NULL_HANDLE;
			VkPipelineStageFlags* waitDstStageMask = nullptr;
		};

		struct PushConstants {
			Matrix4 mvp;
		};

		struct Camera {
			Matrix4 view;
			Matrix4 proj;
			Matrix4 projView;
		};

		struct Material {
			Vector4 color = {1, 1, 1, 1};
			VulkanTexture2D* texture;

			inline static size_t size() {return offsetof(Material, texture);}
		};
	private:
		VulkanSwapchain* m_Swapchain;
		VulkanDevice* m_Device;
		VkRenderPass m_VkRenderPass = VK_NULL_HANDLE;
		VkPipelineLayout m_VkPipelineLayout = VK_NULL_HANDLE;
		VkPipeline m_VkGraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineCache m_VkPipelineCache = VK_NULL_HANDLE;
		VkCommandBuffer m_VkCommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VkFramebuffer m_VkFramebuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VulkanTexture2D* m_DepthTextures[MAX_SWAPCHAIN_IMAGE_COUNT];
		VkDescriptorSetLayout m_VkDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_DescriptorPool;
		VulkanUniformBuffer<Camera>* m_CameraUniformBuffer; // sizeof(Camera) * BATCH_SIZE
		VulkanUniformBuffer<Material>* m_MaterialUniformBuffer; // sizeof(MaterialUniformBuffer) * BATCH_SIZE
		VulkanBuffer* m_VertexBuffer;
		Array<Material, 4> m_Materials = {};
		VulkanSamplerMap* m_Samplers;
		Camera m_Camera = {};
	public:
		explicit VulkanSimpleRenderPass(VulkanSwapchain* swapchain);
		~VulkanSimpleRenderPass();
		void execute(const ExecuteInfo& input);
		void recreate();
	private:
		void create();
		void destroy();
		void createRenderAreaDependentComponents();
		void destroyRenderAreaDependentComponents();
		void updateCameraUniformBuffer(uint32_t swapchainImage, const Camera& camera);
		void updatePushConstants(VkCommandBuffer commandBuffer, const PushConstants& pushConstants);
		void updateMaterialUniformBuffer(uint32_t swapchainImage, const Material& material);
		void createRenderPass();
		void createDepthTextures();
		void createFramebuffers();
		void createCameraUniformBuffer();
		void createMaterialUniformBuffer();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void allocateDescriptorSets();
		void setupDescriptorSet(size_t index, VkDescriptorSet vkDescriptorSet);
		void createPipelineLayout();
		void createGraphicsPipeline();
		void allocateCommandBuffers();
		void createVertexBuffer();
		void createMaterials();
		void createSamplersMap();
	};
}
#pragma once

#include "milo/graphics/rendering/passes/LightCullingPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/rendering/Framebuffer.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"

#define MAX_POINT_LIGHTS 1024

namespace milo {

	class VulkanLightCullingPass : public LightCullingPass {
		friend class LightCullingPass;
	private:
		struct CameraUniformBuffer {
			Matrix4 projectionMatrix;
			Matrix4 viewMatrix;
			Matrix4 viewProjectionMatrix;
		};
		struct PointLightsUniformBuffer {
			uint32_t pointLightsCount;
			PointLight pointLights[MAX_POINT_LIGHTS];
		};
		struct VisibleLightIndicesData {
			int32_t indices[4096];
		};
		struct PushConstants {
			Size screenSize;
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VulkanUniformBuffer<CameraUniformBuffer>* m_CameraUniformBuffer{nullptr};
		VulkanUniformBuffer<PointLightsUniformBuffer>* m_PointLightsUniformBuffer{nullptr};
		Ref<VulkanBuffer> m_VisibleLightsStorageBuffer{nullptr};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool = nullptr;

		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};
		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Size m_LastViewportSize{};
	private:
		VulkanLightCullingPass();
		~VulkanLightCullingPass();
	public:
		bool shouldCompile(Scene* scene) const;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool);
		void execute(Scene* scene);
	private:
		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void updateUniforms(uint32_t imageIndex, Scene* scene);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createCameraUniformBuffer();
		void createPointLightsUniformBuffer();
		void createVisibleLightIndicesStorageBuffer();
		void createDescriptorSets();
		void createComputePipeline();
		void createCommandBuffers();
		void createSemaphores();
	};
}
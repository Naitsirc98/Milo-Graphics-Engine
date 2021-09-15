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
		struct PushConstants {
			Vector2i screenSize;
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VulkanUniformBuffer<CameraUniformBuffer>* m_CameraUniformBuffer{nullptr};
		VulkanUniformBuffer<PointLightsUniformBuffer>* m_PointLightsUniformBuffer{nullptr};
		VulkanBuffer* m_VisibleLightsStorageBuffer{nullptr};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool = nullptr;

		VkPipelineLayout m_PipelineLayout{VK_NULL_HANDLE};
		VkPipeline m_ComputePipeline{VK_NULL_HANDLE};

		Size m_LastFramebufferSize{};
	private:
		VulkanLightCullingPass();
		~VulkanLightCullingPass();
	public:
		bool shouldCompile(Scene* scene) const;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool);
		void execute(Scene* scene);
	private:
		void updateUniforms(Scene* scene);
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createCameraUniformBuffer();
		void createPointLightsUniformBuffer();
		void createVisibleLightIndicesStorageBuffer();
		void createDescriptorSets();
		void createComputePipeline();
	};
}
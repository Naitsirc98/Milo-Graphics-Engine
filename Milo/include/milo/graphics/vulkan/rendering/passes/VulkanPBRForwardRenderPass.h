#pragma once

#include <milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h>
#include "milo/graphics/rendering/passes/PBRForwardRenderPass.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	class VulkanPBRForwardRenderPass : public PBRForwardRenderPass {
		friend class PBRForwardRenderPass;
	private:

		struct CameraUniformBuffer {
			Matrix4 viewMatrix;
			Matrix4 projMatrix;
			Matrix4 projViewMatrix;
			Vector4 position;
		};

		struct LightsUniformBuffer {
			DirectionalLight directionalLight{};
			PointLight pointLights[128]{};
			Vector4 u_AmbientColor{};
			uint pointLightsCount{0};
		};

		struct SkyboxUniformBuffer {
			float maxPrefilterLOD{0};
			float prefilterLODBias{0};
			bool present{false};
		};

		struct PushConstants {
			Matrix4 modelMatrix;
		};

	private:
		VulkanDevice* m_Device = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanUniformBuffer<CameraUniformBuffer>* m_CameraUniformBuffer = nullptr;
		VulkanUniformBuffer<LightsUniformBuffer>* m_LightsUniformBuffer = nullptr;
		VulkanUniformBuffer<SkyboxUniformBuffer>* m_SkyboxUniformBuffer = nullptr;
		VkDescriptorSetLayout m_SceneDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_SceneDescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		VulkanCommandPool* m_CommandPool = nullptr;
		VkCommandBuffer m_CommandBuffers[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

	private:
		VulkanPBRForwardRenderPass();
		~VulkanPBRForwardRenderPass() override;
	public:
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();
		void createFramebuffers(FrameGraphResourcePool* resourcePool);

		void createCameraUniformBuffer();
		void createLightsUniformBuffer();
		void createSkyboxUniformBuffer();
		void createSceneDescriptorLayout();
		void createSceneDescriptorPool();
		void createSceneDescriptorSets();

		void createPipelineLayout();
		void createGraphicsPipeline();

		void createCommandBuffers();

		void createSemaphores();

		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);

		void beginCommandBuffer(VkCommandBuffer commandBuffer) const;
		void beginRenderPass(uint32_t imageIndex, VkCommandBuffer commandBuffer);
		void bindGraphicsPipeline(VkCommandBuffer commandBuffer) const;

		void updateCameraUniformData(uint32_t imageIndex, const Camera* camera, const Transform& cameraTransform);
		void updateLightsUniformData(uint32_t imageIndex, const LightEnvironment& lights);
		void updateSkyboxUniformData(uint32_t imageIndex, const Skybox* skybox);
		void bindSceneDescriptorSet(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void renderScene(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) const;
	};
}
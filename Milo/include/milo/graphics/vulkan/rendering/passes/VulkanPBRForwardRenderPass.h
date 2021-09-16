#pragma once

#include <milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h>
#include "milo/graphics/rendering/passes/PBRForwardRenderPass.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	class VulkanPBRForwardRenderPass : public PBRForwardRenderPass {
		friend class PBRForwardRenderPass;
	private:
		// ===================================== SET 0
		struct CameraData {
			Matrix4 viewMatrix{};
			Matrix4 projViewMatrix{};
		};

		struct RendererData {
			Matrix4 u_LightMatrix[4]{};
			Vector4 u_CascadeSplits{};
			int u_TilesCountX{};
			bool u_ShowCascades{};
			bool u_SoftShadows{};
			float u_LightSize{};
			float u_MaxShadowDistance{};
			float u_ShadowFade{};
			bool u_CascadeFading{};
			float u_CascadeTransitionFade{};
			bool u_ShowLightComplexity{};
		};

		struct SceneData {
			DirectionalLight u_DirectionalLights{};
			Vector3 u_CameraPosition{};
			uint32_t u_PointLightsCount{};
			PointLight u_pointLights[1024]{};
			float u_EnvironmentMapIntensity{};
		};

		struct VisibleLightsIndices {
			int32_t indices[4096];
		};

		// ====================================== SET 1 = MATERIAL

		// ====================================== SET 2 = Skybox textures

		// ====================================== SET 3 = Shadow maps

		// ====================================== PUSH CONSTANTS
		struct PushConstants {
			Matrix4 modelMatrix;
		};

	private:
		VulkanDevice* m_Device = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanUniformBuffer<CameraData>* m_CameraUniformBuffer = nullptr;
		VulkanUniformBuffer<RendererData>* m_RenderUniformBuffer = nullptr;
		VulkanUniformBuffer<SceneData>* m_SceneUniformBuffer = nullptr;

		VkDescriptorSetLayout m_SceneDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_SceneDescriptorPool = nullptr;

		VkDescriptorSetLayout m_SkyboxDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_SkyboxDescriptorPool = nullptr;

		VkDescriptorSetLayout m_ShadowsDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_ShadowsDescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

	private:
		VulkanPBRForwardRenderPass();
		~VulkanPBRForwardRenderPass() override;
	public:
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void renderScene(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);

		void drawMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const;
		void pushConstants(VkCommandBuffer commandBuffer, const Matrix4& transform) const;
		void bindMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const;
		void bindMaterial(VkCommandBuffer commandBuffer, const VulkanMaterialResourcePool& materialResources, Material* material) const;

		void updateCameraUniformData(uint32_t imageIndex);
		void updateRendererDataUniformData(uint32_t imageIndex);
		void updateSceneDataUniformData(uint32_t imageIndex);
		void updateSkyboxUniformData(uint32_t imageIndex, const Skybox* skybox);

		void bindSceneDescriptorSet(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void createRenderPass();

		void createCameraUniformBuffer();
		void createRendererDataUniformBuffer();
		void createSceneDataUniformBuffer();

		void createSceneDescriptorLayoutAndPool();
		void createSceneDescriptorSets();

		void createSkyboxDescriptorLayoutAndPool();
		void createSkyboxDescriptorSets();

		void createShadowsDescriptorLayoutAndPool();
		void createShadowsDescriptorSets();

		void createGraphicsPipeline();
		void createSemaphores();
	};
}
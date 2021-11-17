#pragma once

#include <milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h>
#include "milo/graphics/rendering/passes/PBRForwardRenderPass.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include <concurrent_queue.h>

namespace milo {

	class VulkanPBRForwardRenderPass : public PBRForwardRenderPass {
		friend class PBRForwardRenderPass;
	private:
		// ===================================== SET 0: SCENE

		struct CameraData {
			Matrix4 viewProjection{};
			Matrix4 view{};
			Vector4 position{};
		};

		struct EnvironmentData {
			DirectionalLight dirLight{};
			bool dirLightPresent[4]{false};
			float maxPrefilterLod{0};
			float prefilterLodBias{0};
			bool skyboxPresent[4]{false};
		};

		struct PointLightsData {
			PointLight pointLights[MAX_POINT_LIGHTS]{};
			uint32_t u_PointLightsCount{0};
		};

		// =============================================

		struct ShadowDetails {
			Matrix4 u_LightMatrix[4]{};
			Vector4 u_CascadeSplits{};
			uint32_t u_TilesCountX{};
			bool u_ShowCascades[4]{};
			bool u_SoftShadows[4]{};
			float u_LightSize{};
			float u_MaxShadowDistance{};
			float u_ShadowFade{};
			bool u_CascadeFading[4]{};
			float u_CascadeTransitionFade{};
			bool u_ShowLightComplexity[4]{};
			bool u_ShadowsEnabled[4]{};
		};

		// =============================================

		struct PushConstants {
			Matrix4 modelMatrix;
		};

	private:
		VulkanDevice* m_Device = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanUniformBuffer<CameraData>* m_CameraUniformBuffer = nullptr;
		VulkanUniformBuffer<EnvironmentData>* m_EnvironmentUniformBuffer = nullptr;
		VulkanUniformBuffer<PointLightsData>* m_PointLightsUniformBuffer = nullptr;

		VkDescriptorSetLayout m_SceneDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_SceneDescriptorPool = nullptr;

		VulkanUniformBuffer<ShadowDetails>* m_ShadowsUniformBuffer = nullptr;

		VkDescriptorSetLayout m_ShadowsDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_ShadowsDescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Array<uint32_t, MAX_SWAPCHAIN_IMAGE_COUNT> m_LastSkyboxModificationCount{0};

	public:
		VulkanPBRForwardRenderPass();
		~VulkanPBRForwardRenderPass() override;
	public:
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer);
		void renderScene(uint32_t imageIndex, VkCommandBuffer commandBuffer);

		void drawMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const;
		void pushConstants(VkCommandBuffer commandBuffer, const Matrix4& transform) const;
		void bindMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const;
		void bindMaterial(VkCommandBuffer commandBuffer, const VulkanMaterialResourcePool& materialResources, Material* material) const;

		void updateSceneUniformData(uint32_t imageIndex);
		void setSkyboxUniformData(uint32_t imageIndex, Skybox* skybox);
		void setNullSkyboxUniformData(uint32_t imageIndex);

		void updateShadowsUniformData(uint32_t imageIndex, uint32_t viewportWidth);

		void bindDescriptorSets(uint32_t imageIndex, VkCommandBuffer commandBuffer);

		void createRenderPass();

		void createSceneUniformBuffers();
		void createSceneDescriptorSetLayoutAndPool();

		void createShadowsUniformBuffer();
		void createShadowsDescriptorSetLayoutAndPool();

		void createGraphicsPipeline();
		void createSemaphores();

		void renderSceneSingleThread(uint32_t imageIndex, VkCommandBuffer commandBuffer,
									 const VulkanMaterialResourcePool& materialResources);
	};
}

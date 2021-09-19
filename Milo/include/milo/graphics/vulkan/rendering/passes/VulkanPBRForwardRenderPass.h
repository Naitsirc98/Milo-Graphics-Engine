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
		// ===================================== SET 0: SCENE

		struct CameraData {
			Matrix4 viewProjection{};
			Matrix4 view{};
			Vector4 position{};
		};

		struct EnvironmentData {
			DirectionalLight dirLight{};
			PointLight pointLights[1]{};
			uint32_t u_PointLightsCount{0};
			bool dirLightPresent{false};
			float maxPrefilterLod{0};
			float prefilterLodBias{0};
			bool skyboxPresent{false};
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

		VkDescriptorSetLayout m_SceneDescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_SceneDescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Array<uint32_t, MAX_SWAPCHAIN_IMAGE_COUNT> m_LastSkyboxModificationCount{0};

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

		void updateSceneUniformData(uint32_t imageIndex, Scene* scene);
		void setSkyboxUniformData(uint32_t imageIndex, Skybox* skybox);
		void setNullSkyboxUniformData(uint32_t imageIndex);

		void bindDescriptorSets(uint32_t imageIndex, VkCommandBuffer commandBuffer);

		void createRenderPass();

		void createCameraUniformBuffer();
		void createEnvironmentDataUniformBuffer();

		void createSceneDescriptorSetLayoutAndPool();

		void createGraphicsPipeline();
		void createSemaphores();
	};
}
#pragma once

#include "MaterialViewerRenderer.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanPBRForwardRenderPass.h"
#include "milo/graphics/vulkan/rendering/passes/VulkanSkyboxRenderPass.h"
#include "milo/graphics/rendering/Framebuffer.h"
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"

namespace milo {

	struct MaterialViewerCameraData {
		Matrix4 viewProjection{};
		Matrix4 proj{};
		Matrix4 view{};
		Vector4 position{};
	};

	class VulkanMaterialPBRRenderer {
	private:
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

		// =============================================

		struct PushConstants {
			Matrix4 modelMatrix;
		};

	private:
		MaterialViewerCameraData* m_Camera;

		Material* m_Material{nullptr};
		uint32_t m_LastVersion{0};

		VulkanDevice* m_Device = nullptr;

		VkRenderPass m_RenderPass = VK_NULL_HANDLE;

		VulkanUniformBuffer<CameraData>* m_CameraUniformBuffer = nullptr;
		VulkanUniformBuffer<EnvironmentData>* m_EnvironmentUniformBuffer = nullptr;

		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
		VulkanDescriptorPool* m_DescriptorPool = nullptr;

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		VkCommandBuffer m_CommandBuffer{VK_NULL_HANDLE};

		VulkanFramebuffer* m_Framebuffer{nullptr};

	public:
		VulkanMaterialPBRRenderer(VulkanFramebuffer* framebuffer, MaterialViewerCameraData* camera);
		~VulkanMaterialPBRRenderer();
		void render(Material* material);
	private:
		void buildCommandBuffer();
		void renderMaterial();
		void bindDescriptorSets();
		void createRenderPass();
		void createUniformBuffers();
		void createDescriptorSetLayoutAndPool();
		void createGraphicsPipeline();
	};

	class VulkanMaterialSkyboxRenderer {
	private:
		struct UniformBuffer {
			Matrix4 viewMatrix;
			Matrix4 projMatrix;
			float textureLOD;
			float intensity;
		};
	private:
		MaterialViewerCameraData* m_Camera;

		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VulkanUniformBuffer<UniformBuffer>* m_UniformBuffer{nullptr};

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		VulkanFramebuffer* m_Framebuffer = nullptr;
	public:
		VulkanMaterialSkyboxRenderer(VulkanFramebuffer* framebuffer, MaterialViewerCameraData* camera);
		~VulkanMaterialSkyboxRenderer();
		void render();
	private:
		void createRenderPass();
		void createUniformBuffers();
		void createDescriptorSetLayoutAndPool();
		void createGraphicsPipeline();
		void bindDescriptorSets();
		void updateDescriptorSets(Skybox* skybox, VkDescriptorSet descriptorSet);
		void buildCommandBuffer();
	};

	class VulkanMaterialViewerRenderer : public MaterialViewerRenderer {
	private:
		MaterialViewerCameraData m_Camera;
		VulkanFramebuffer* m_Framebuffer;
		VulkanMaterialPBRRenderer* m_MaterialPBRRenderer;
		VulkanMaterialSkyboxRenderer* m_MaterialSkyboxRenderer;
	public:
		VulkanMaterialViewerRenderer();
		~VulkanMaterialViewerRenderer() override;
	public:
		const Texture2D& render(Material* material) override;
	};
}

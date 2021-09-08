#pragma once

#include "milo/graphics/rendering/passes/SkyboxRenderPass.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"

namespace milo {

	class VulkanSkyboxRenderPass : public SkyboxRenderPass {
		friend class SkyboxRenderPass;
	public:
		struct UniformBuffer {
			Matrix4 viewMatrix;
			Matrix4 projMatrix;
			float textureLOD;
			float intensity;
		};
	private:
		VulkanDevice* m_Device{nullptr};

		VkRenderPass m_RenderPass{VK_NULL_HANDLE};

		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		VulkanUniformBuffer<UniformBuffer>* m_UniformBuffer{nullptr};

		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;

		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};

		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};

		Size m_LastFramebufferSize{};

	private:
		VulkanSkyboxRenderPass();
		~VulkanSkyboxRenderPass() override;
	public:
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void updateDescriptorSets(Skybox* skybox, uint32_t imageIndex, VkDescriptorSet descriptorSet);
		void createRenderPass();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createUniformBuffer();
		void createDescriptorSets();
		void createGraphicsPipeline();
		void createCommandBuffers(FrameGraphResourcePool* resourcePool);
		void createSemaphores();
		void destroyTransientResources();
	};

}
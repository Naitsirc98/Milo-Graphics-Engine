#pragma once

#include <milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h>
#include "milo/graphics/rendering/passes/GridRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/buffers/VulkanUniformBuffer.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	class VulkanGridRenderPass : public GridRenderPass {
	private:
		struct UniformBuffer {
			Matrix4 projViewModel;
			float scale;
			float size;
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
	public:
		VulkanGridRenderPass();
		~VulkanGridRenderPass();
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void createRenderPass();

		void createUniformBuffer();
		void createDescriptorSetLayout();
		void createDescriptorPool();
		void createDescriptorSets();

		void createGraphicsPipeline();

		void createCommandPool();
		void createCommandBuffers(FrameGraphResourcePool* resourcePool);

		void createSemaphores();
	};
}
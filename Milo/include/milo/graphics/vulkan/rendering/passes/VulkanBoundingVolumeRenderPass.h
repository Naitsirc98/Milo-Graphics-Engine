#pragma once

#include "milo/graphics/rendering/passes/BoundingVolumeRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	class VulkanBoundingVolumeRenderPass : public BoundingVolumeRenderPass {
	private:
		struct PushConstants {
			Matrix4 projViewModel;
			Color color;
		};
	private:
		VulkanDevice* m_Device{nullptr};
		VkRenderPass m_RenderPass{VK_NULL_HANDLE};
		VulkanGraphicsPipeline* m_GraphicsPipeline = nullptr;
		Array<VkCommandBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> m_CommandBuffers{};
		Array<VkSemaphore, MAX_SWAPCHAIN_IMAGE_COUNT> m_SignalSemaphores{};
	public:
		VulkanBoundingVolumeRenderPass();
		~VulkanBoundingVolumeRenderPass();
		bool shouldCompile(Scene* scene) const override;
		void compile(Scene* scene, FrameGraphResourcePool* resourcePool) override;
		void execute(Scene* scene) override;
	private:
		void buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene);
		void createRenderPass();
		void createGraphicsPipeline();
	};

}
#pragma once

#include <milo/graphics/vulkan/rendering/VulkanSimpleRenderPass.h>
#include "milo/graphics/rendering/GraphicsPresenter.h"
#include "milo/graphics/vulkan/images/VulkanTexture.h"
#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "VulkanSwapchain.h"

namespace milo {

	class VulkanPresenter : public GraphicsPresenter {
		friend class GraphicsPresenter;
		friend class VulkanContext;
	private:
		VulkanContext& m_Context;
		VulkanDevice& m_Device;
		VulkanSwapchain& m_Swapchain;
		// Queues
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentationQueue = VK_NULL_HANDLE;
		// Synchronization objects
		VkFence m_ImageAvailableFences[MAX_SWAPCHAIN_IMAGE_COUNT]{VK_NULL_HANDLE};
		VkFence m_FramesInFlightFences[MAX_FRAMES_IN_FLIGHT]{VK_NULL_HANDLE};
		VkSemaphore m_ImageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT]{VK_NULL_HANDLE};
		VkSemaphore m_RenderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT]{VK_NULL_HANDLE};
		// Current State
		uint32_t m_MaxImageCount = 0;
		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrame = 0;
		// TMP
		VulkanSimpleRenderPass* m_SimpleRenderPass = nullptr;
	private:
		explicit VulkanPresenter(VulkanContext& context);
		~VulkanPresenter();
	public:
		bool begin() override;
		void end() override;
		[[nodiscard]] uint32_t currentImageIndex() const;
		[[nodiscard]] uint32_t maxImageCount() const;
	private:
		void waitForPreviousFrameToComplete();
		bool tryGetNextSwapchainImage();
		void setCurrentFrameInFlight();
		void createSyncObjects();
		void destroySyncObjects();
		uint32_t advanceToNextFrame();
	};
}
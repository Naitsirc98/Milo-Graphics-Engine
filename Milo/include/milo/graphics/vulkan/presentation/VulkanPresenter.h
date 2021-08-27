#pragma once

#include "milo/graphics/rendering/GraphicsPresenter.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "VulkanSwapchain.h"

namespace milo {

	class VulkanPresenter : public GraphicsPresenter {
		friend class GraphicsPresenter;
		friend class VulkanContext;
	private:
		VulkanDevice* m_Device;
		VulkanSwapchain* m_Swapchain;
		// Queues
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentationQueue = VK_NULL_HANDLE;
		// Synchronization objects
		Array<VkFence, MAX_SWAPCHAIN_IMAGE_COUNT> m_ImageAvailableFences{};
		Array<VkFence, MAX_FRAMES_IN_FLIGHT> m_FramesInFlightFences{};
		Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_ImageAvailableSemaphore{};
		Array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_RenderFinishedSemaphore{};
		// Current State
		uint32_t m_MaxImageCount = 0;
		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrame = -1;
	private:
		explicit VulkanPresenter(VulkanContext* context);
	public:
		~VulkanPresenter();
		bool begin() override;
		void end() override;
		uint32_t currentImageIndex() const;
		uint32_t maxImageCount() const;
		uint32_t currentFrame() const;
		VkFence frameInFlightFence() const;
		VkSemaphore imageAvailableSemaphore() const;
		VkSemaphore renderFinishedSemaphore() const;
	private:
		void waitForPreviousFrameToComplete();
		bool tryGetNextSwapchainImage();
		void setCurrentFrameInFlight();
		void createSyncObjects();
		void destroySyncObjects();
		uint32_t advanceToNextFrame();
	};
}
#include "milo/graphics/vulkan/presentation/VulkanPresenter.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	VulkanPresenter::VulkanPresenter(VulkanContext* context)
			: m_Device(context->device()), m_Swapchain(context->swapchain()) {

		m_GraphicsQueue = m_Device->graphicsQueue()->vkQueue();
		m_PresentationQueue = m_Device->presentationQueue()->vkQueue();

		m_MaxImageCount = m_Swapchain->imageCount();
		m_CurrentImageIndex = 0;
		m_CurrentFrame = 0;

		createSyncObjects();

		m_Swapchain->addSwapchainRecreateCallback([&]() {
			m_MaxImageCount = m_Swapchain->imageCount();
			m_CurrentImageIndex = 0;
			m_CurrentFrame = 0;

			destroySyncObjects();
			createSyncObjects();
		});
	}

	VulkanPresenter::~VulkanPresenter() {
		destroySyncObjects();
	}

	bool VulkanPresenter::begin() {

		if(Window::get()->size().isZero()) return false;

		m_CurrentFrame = advanceToNextFrame();

		waitForPreviousFrameToComplete();

		if(!tryGetNextSwapchainImage()) return false;

		setCurrentFrameInFlight();

		return true;
	}

	inline void VulkanPresenter::waitForPreviousFrameToComplete() {
		VK_CALL(vkWaitForFences(m_Device->logical(), 1, &m_FramesInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX));
	}

	inline bool VulkanPresenter::tryGetNextSwapchainImage() {

		VkDevice dv = m_Device->logical();
		VkSwapchainKHR swch = m_Swapchain->vkSwapchain();
		VkSemaphore imgSemaphore = m_ImageAvailableSemaphore[m_CurrentFrame];

		VkResult result = VK_CALLR(vkAcquireNextImageKHR(dv, swch, UINT64_MAX, imgSemaphore, VK_NULL_HANDLE, &m_CurrentImageIndex));

		bool success;

		switch(result) {
			case VK_SUCCESS:
				success = true;
				break;
			case VK_NOT_READY:
				Log::warn(str("vkAcquireNextImageKHR returned ") + mvk::getErrorName(result));
				success = false;
				break;
			case VK_SUBOPTIMAL_KHR:
			case VK_ERROR_OUT_OF_DATE_KHR:
				m_Swapchain->recreate();
				success = false;
				break;
			default:
				throw MILO_RUNTIME_EXCEPTION(str("Failed to acquire swapchain image: ") + mvk::getErrorName(result));
		}

		if (m_ImageAvailableFences[m_CurrentImageIndex] != VK_NULL_HANDLE)
			VK_CALL(vkWaitForFences(dv, 1, &m_ImageAvailableFences[m_CurrentImageIndex], VK_TRUE, UINT64_MAX));

		m_ImageAvailableFences[m_CurrentImageIndex] = m_FramesInFlightFences[m_CurrentFrame];

		return success;
	}

	inline void VulkanPresenter::setCurrentFrameInFlight() {
		VK_CALL(vkResetFences(m_Device->logical(), 1, &m_FramesInFlightFences[m_CurrentFrame]));
	}

	void VulkanPresenter::end() {

		VkSwapchainKHR swapchains = m_Swapchain->vkSwapchain();

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.pWaitSemaphores = m_Device->graphicsQueue()->waitSemaphores().data();
		presentInfo.waitSemaphoreCount = m_Device->graphicsQueue()->waitSemaphores().size();
		//presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore[m_CurrentFrame];
		//presentInfo.waitSemaphoreCount = 1;
		presentInfo.pSwapchains = &swapchains;
		presentInfo.swapchainCount = 1;
		presentInfo.pImageIndices = &m_CurrentImageIndex;

		VkResult result = VK_CALLR(vkQueuePresentKHR(m_PresentationQueue, &presentInfo));

		switch(result) {
			case VK_SUCCESS:
				break;
			case VK_SUBOPTIMAL_KHR:
			case VK_ERROR_OUT_OF_DATE_KHR:
				m_Swapchain->recreate();
				break;
			default:
				throw MILO_RUNTIME_EXCEPTION(str("Failed to acquire swapchain image: ") + mvk::getErrorName(result));
		}
	}

	uint32_t VulkanPresenter::currentImageIndex() const {
		return m_CurrentImageIndex;
	}

	uint32_t VulkanPresenter::maxImageCount() const {
		return m_MaxImageCount;
	}

	uint32_t VulkanPresenter::currentFrame() const {
		return m_CurrentFrame;
	}

	void VulkanPresenter::createSyncObjects() {

		VkDevice device = m_Device->logical();

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		memset(m_ImageAvailableFences, NULL, MAX_SWAPCHAIN_IMAGE_COUNT * sizeof(VkFence));

		for(uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i) {
			VK_CALL(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore[i]));
			VK_CALL(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore[i]));
			VK_CALL(vkCreateFence(device, &fenceInfo, nullptr, &m_FramesInFlightFences[i]));
		}
	}

	void VulkanPresenter::destroySyncObjects() {
		VkDevice device = m_Device->logical();
		for(uint32_t i = 0;i < MAX_FRAMES_IN_FLIGHT;++i) {
			VK_CALLV(vkDestroySemaphore(device, m_ImageAvailableSemaphore[i], nullptr));
			VK_CALLV(vkDestroySemaphore(device, m_RenderFinishedSemaphore[i], nullptr));
			VK_CALLV(vkDestroyFence(device, m_FramesInFlightFences[i], nullptr));
		}
		memset(m_ImageAvailableFences, NULL, MAX_SWAPCHAIN_IMAGE_COUNT * sizeof(VkFence));
	}

	uint32_t VulkanPresenter::advanceToNextFrame() {
		return (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}
}
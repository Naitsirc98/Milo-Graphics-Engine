#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/presentation/VulkanPresenter.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	VulkanFrameGraphResourcePool::VulkanFrameGraphResourcePool() {
	}

	VulkanFrameGraphResourcePool::~VulkanFrameGraphResourcePool() {
	}

	uint32_t VulkanFrameGraphResourcePool::currentFramebufferIndex() const {
		return VulkanContext::get()->vulkanPresenter()->currentImageIndex();
	}

	uint32_t VulkanFrameGraphResourcePool::maxDefaultFramebuffersCount() const {
		return VulkanContext::get()->swapchain()->imageCount();
	}
}
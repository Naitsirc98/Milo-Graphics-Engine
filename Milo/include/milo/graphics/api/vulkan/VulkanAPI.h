#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "milo/common/Common.h"

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_SWAPCHAIN_IMAGE_COUNT 3

namespace milo {
	String vulkanErrorName(VkResult vkResult) noexcept;
}

#ifdef _DEBUG
#define VK_CALL(func) {VkResult vkResult = func; if(vkResult != VK_SUCCESS) throw MILO_RUNTIME_EXCEPTION(milo::str("Error when invoking ").append(#func).append(": ").append(milo::vulkanErrorName(vkResult)));}
#else
#define VK_CALL(func) func
#endif


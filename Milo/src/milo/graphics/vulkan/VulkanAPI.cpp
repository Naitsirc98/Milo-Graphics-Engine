#include "milo/graphics/vulkan/VulkanAPI.h"

namespace milo {

	String vulkanErrorName(VkResult vkResult) noexcept {
		switch (vkResult) {
			case VK_SUCCESS: return "VK_SUCCESS";
			case VK_NOT_READY: return "VK_NOT_READY";
			case VK_TIMEOUT: return "VK_TIMEOUT";
			case VK_EVENT_SET: return "VK_EVENT_SET";
			case VK_EVENT_RESET: return "VK_EVENT_RESET";
			case VK_INCOMPLETE: return "VK_INCOMPLETE";
			case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
			case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
			case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
			case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
			case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
			case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
			case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
			case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
			case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
			case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
			case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
			case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
			case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
			case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
			case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
			case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
			case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
			case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
			case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
			case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
			case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
			case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
			case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
			case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		}
		return "Unknown Error";
	}

	VulkanAPICall VulkanAPICallManager::s_LastVulkanAPICall;
	Mutex VulkanAPICallManager::s_Mutex;

	VulkanAPICall VulkanAPICallManager::popVkCall() {
		return s_LastVulkanAPICall;
	}

	void VulkanAPICallManager::pushVkCall(StackTrace&& stackTrace, const char* function, const char* file, uint32_t line) {
		s_Mutex.lock();
		{
			s_LastVulkanAPICall.stacktrace = std::forward<StackTrace>(stackTrace);
			s_LastVulkanAPICall.function = function;
			s_LastVulkanAPICall.file = file;
			s_LastVulkanAPICall.line = line;
		}
		s_Mutex.unlock();
	}

	VkResult VulkanAPICallManager::pushVkCall(StackTrace&& stackTrace, const char* function, const char* file,
											  uint32_t line, VkResult vkResult) {

		pushVkCall(std::forward<StackTrace>(stackTrace), function, file, line);
		return vkResult;
	}

	VkImageMemoryBarrier mvk::vkImageMemoryBarrier(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) noexcept {

		// GENERIC, MAY BE CHANGED BY CALLER
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.subresourceRange = range;

		barrier.image = image;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;

		// SHOULD BE SET BY CALLER IF OTHER BEHAVIOUR IS DESIRED
		if(oldLayout != newLayout) {
			if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			} else {
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			}
		}

		return barrier;
	}
}
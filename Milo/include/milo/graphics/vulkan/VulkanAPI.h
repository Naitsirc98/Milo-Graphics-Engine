#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "milo/graphics/Graphics.h"
#include "milo/assets/images/PixelFormat.h"

#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_SWAPCHAIN_IMAGE_COUNT 3

namespace milo {

	struct VulkanAPICall {
		StackTrace stacktrace;
		String function = "UNKNOWN";
		String file = "UNKNOWN";
		uint32_t line = 0;
	};

	class VulkanAPICallManager {
	private:
		static VulkanAPICall s_LastVulkanAPICall;
		static Mutex s_Mutex;
	public:
		static VulkanAPICall popVkCall();
		static void pushVkCall(StackTrace&& stackTrace, const char* function, const char* file, uint32_t line);
		static VkResult pushVkCall(StackTrace&& stackTrace, const char* function, const char* file, uint32_t line, VkResult vkResult);
	};

	namespace mvk {

		namespace ImageMemoryBarrier {
			VkImageMemoryBarrier create(VkImage image = VK_NULL_HANDLE,
										VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
										VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED) noexcept;
		}

		namespace ImageCreateInfo {
			VkImageCreateInfo create() noexcept;
			VkImageCreateInfo colorAttachment() noexcept;
			VkImageCreateInfo depthStencilAttachment() noexcept;
		}

		namespace ImageViewCreateInfo {
			VkImageViewCreateInfo create(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
			VkImageViewCreateInfo colorAttachment(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
			VkImageViewCreateInfo depthStencilAttachment(VkImage image = VK_NULL_HANDLE, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, uint32_t levelCount = 1) noexcept;
		}

		namespace SamplerCreateInfo {
			VkSamplerCreateInfo create() noexcept;
		}

		namespace BufferCreateInfo {
			VkBufferCreateInfo create(VkBufferUsageFlags usage = 0) noexcept;
		}

		namespace FramebufferCreateInfo {
			VkFramebufferCreateInfo create(VkRenderPass renderPass = VK_NULL_HANDLE, uint32_t width = 0, uint32_t height = 0) noexcept;
		}

		String getErrorName(VkResult vkResult) noexcept;
		PixelFormat toPixelFormat(VkFormat format);
		VkFormat fromPixelFormat(PixelFormat pixelFormat);
	}
}

#ifdef _DEBUG

#define VK_CALL(func) {milo::VulkanAPICallManager::pushVkCall(milo::getStackTrace(), #func, __FILE__, __LINE__); VkResult vkResult = func; if(vkResult != VK_SUCCESS) \
throw MILO_RUNTIME_EXCEPTION(milo::str("Error when invoking ").append(#func).append(": ").append(milo::mvk::getErrorName(vkResult)));}
#define VK_CALLV(func) milo::VulkanAPICallManager::pushVkCall(milo::getStackTrace(), #func, __FILE__, __LINE__); func;
#define VK_CALLR(func) milo::VulkanAPICallManager::pushVkCall(milo::getStackTrace(), #func, __FILE__, __LINE__, func);
#else
#define VK_CALL(func) func
#define VK_CALLV(func) func
#define VK_CALLR(func) func
#endif


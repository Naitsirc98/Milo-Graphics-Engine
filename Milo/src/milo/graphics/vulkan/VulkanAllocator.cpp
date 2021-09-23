#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/Graphics.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace milo {

	struct AllocationInfo {
		String type;
		StackTrace stacktrace;
	};

	static HashMap<VmaAllocation, AllocationInfo> g_Allocations;

	VulkanAllocator* VulkanAllocator::get() {
		return VulkanContext::get()->allocator();
	}

	VulkanAllocator::VulkanAllocator(VulkanContext* context) : m_Context(context) {

		VmaAllocatorCreateInfo createInfo = {};
		createInfo.instance = m_Context->vkInstance();
		createInfo.physicalDevice = m_Context->device()->physical();
		createInfo.device = m_Context->device()->logical();
		createInfo.frameInUseCount = MAX_FRAMES_IN_FLIGHT;
		createInfo.vulkanApiVersion = VK_MAKE_VERSION(1, 2, 0);

		VK_CALL(vmaCreateAllocator(&createInfo, &m_VmaAllocator));
	}

	VulkanAllocator::~VulkanAllocator() {

		if(!g_Allocations.empty()) {
			Log::error("[VULKAN ALLOCATOR] The following allocations ({}) have not been released:", g_Allocations.size());
			uint32_t i = 1;
			for(const auto& [allocation, allocInfo] : g_Allocations) {
				Log::error("  {} allocation 0x{:x}:\n{}:\n", allocInfo.type, (uint64_t)allocation, str(allocInfo.stacktrace));
			}
		}

		VK_CALLV(vmaDestroyAllocator(m_VmaAllocator));
		m_VmaAllocator = nullptr;
	}

	VmaAllocator VulkanAllocator::vmaAllocator() const {
		return m_VmaAllocator;
	}

	void VulkanAllocator::mapMemory(VmaAllocation allocation, void** data) {
		VK_CALL(vmaMapMemory(m_VmaAllocator, allocation, data));
	}

	void VulkanAllocator::unmapMemory(VmaAllocation allocation) {
		VK_CALLV(vmaUnmapMemory(m_VmaAllocator, allocation));
	}

	void VulkanAllocator::flushMemory(VmaAllocation allocation, uint64_t offset, uint64_t size) {
		VK_CALL(vmaFlushAllocation(m_VmaAllocator, allocation, offset, size));
	}

	void VulkanAllocator::allocateBuffer(const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo, VkBuffer& vkBuffer, VmaAllocation& vmaAllocation) {
		VK_CALL(vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &allocInfo, &vkBuffer, &vmaAllocation, nullptr));
		g_Allocations[vmaAllocation] = {"Buffer", getStackTrace()};
	}

	void VulkanAllocator::freeBuffer(VkBuffer vkBuffer, VmaAllocation vmaAllocation) {
		g_Allocations.erase(vmaAllocation);
		VK_CALLV(vmaDestroyBuffer(m_VmaAllocator, vkBuffer, vmaAllocation));
	}

	void VulkanAllocator::allocateImage(const VkImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo,
										VkImage& vkImage, VmaAllocation& vmaAllocation) {
		VK_CALL(vmaCreateImage(m_VmaAllocator, &imageInfo, &allocInfo, &vkImage, &vmaAllocation, nullptr));
		g_Allocations[vmaAllocation] = {"Image", getStackTrace()};
	}

	void VulkanAllocator::freeImage(VkImage vkImage, VmaAllocation vmaAllocation) {
		g_Allocations.erase(vmaAllocation);
		VK_CALLV(vmaDestroyImage(m_VmaAllocator, vkImage, vmaAllocation));
	}
}
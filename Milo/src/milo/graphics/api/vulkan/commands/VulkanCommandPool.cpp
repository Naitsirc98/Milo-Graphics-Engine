#include "milo/graphics/api/vulkan/commands/VulkanCommandPool.h"

namespace milo {

	VulkanCommandPool::VulkanCommandPool(const VulkanQueue& queue) : m_Queue(queue) {

		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = queue.family;
		createInfo.flags = 0;

		VK_CALL(vkCreateCommandPool(queue.device->ldevice(), &createInfo, nullptr, &m_VkCommandPool));
	}

	VulkanCommandPool::~VulkanCommandPool() {
		vkDestroyCommandPool(m_Queue.device->ldevice(), m_VkCommandPool, nullptr);
		m_VkCommandPool = VK_NULL_HANDLE;
	}

	VkCommandPool VulkanCommandPool::vkCommandPool() const {
		return m_VkCommandPool;
	}

	const VulkanQueue& VulkanCommandPool::queue() const {
		return m_Queue;
	}

	void VulkanCommandPool::allocate(VkCommandBufferLevel level, uint32_t count, VkCommandBuffer* commandBuffers) {

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.commandPool = m_VkCommandPool;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.level = level;
		allocateInfo.commandBufferCount = count;

		VK_CALL(vkAllocateCommandBuffers(m_Queue.device->ldevice(), &allocateInfo, commandBuffers));
	}

	void VulkanCommandPool::free(uint32_t count, VkCommandBuffer* commandBuffers) {
		vkFreeCommandBuffers(m_Queue.device->ldevice(), m_VkCommandPool, count, commandBuffers);
	}
}
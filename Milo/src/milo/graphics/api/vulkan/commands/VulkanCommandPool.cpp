#include "milo/graphics/api/vulkan/commands/VulkanCommandPool.h"

namespace milo {

	VulkanCommandPool::VulkanCommandPool(const VulkanQueue& queue) : m_Queue(queue), m_VkCommandPool(VK_NULL_HANDLE) {
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = m_Queue.family;
		createInfo.flags = 0;

		VK_CALL(vkCreateCommandPool(m_Queue.device->ldevice(), &createInfo, nullptr, &m_VkCommandPool));
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

	void VulkanCommandPool::execute(const VulkanTask& task) {

		VkCommandBuffer commandBuffer;
		allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pInheritanceInfo = nullptr;
		beginInfo.flags = 0;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			task.run(commandBuffer);
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitSemaphores = task.waitSemaphores;
		submitInfo.waitSemaphoreCount = task.waitSemaphoresCount;
		submitInfo.pSignalSemaphores = task.signalSemaphores;
		submitInfo.signalSemaphoreCount = task.signalSemaphoresCount;

		VK_CALL(vkQueueSubmit(m_Queue.vkQueue, 1, &submitInfo, task.fence));

		if (!task.asynchronous) {
			VK_CALL(vkQueueWaitIdle(m_Queue.vkQueue));
		}
	}
}
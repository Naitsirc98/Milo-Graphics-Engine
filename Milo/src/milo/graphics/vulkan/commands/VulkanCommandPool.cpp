#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"

namespace milo {

	VulkanCommandPool::VulkanCommandPool(const VulkanQueue& queue, VkCommandPoolCreateFlags flags) : m_Queue(queue), m_VkCommandPool(VK_NULL_HANDLE) {
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.queueFamilyIndex = m_Queue.family;
		createInfo.flags = flags;

		VK_CALL(vkCreateCommandPool(m_Queue.device->ldevice(), &createInfo, nullptr, &m_VkCommandPool));
	}

	VulkanCommandPool::~VulkanCommandPool() {
		VK_CALLV(vkDestroyCommandPool(m_Queue.device->ldevice(), m_VkCommandPool, nullptr));
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
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = m_VkCommandPool;
		allocateInfo.level = level;
		allocateInfo.commandBufferCount = count;

		VK_CALL(vkAllocateCommandBuffers(m_Queue.device->ldevice(), &allocateInfo, commandBuffers));
	}

	void VulkanCommandPool::free(uint32_t count, VkCommandBuffer* commandBuffers) {
		VK_CALLV(vkFreeCommandBuffers(m_Queue.device->ldevice(), m_VkCommandPool, count, commandBuffers));
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
		submitInfo.pWaitDstStageMask = task.waitDstStageMask;

		if(task.asynchronous) {

			VK_CALL(vkQueueSubmit(m_Queue.vkQueue, 1, &submitInfo, task.fence));

		} else {

			VkFence fence = task.fence;
			bool shouldDeleteFence = false;

			if(fence == VK_NULL_HANDLE) {
				VkFenceCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				//createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				VK_CALL(vkCreateFence(m_Queue.device->ldevice(), &createInfo, nullptr, &fence));
				shouldDeleteFence = true;
			}

			VK_CALL(vkQueueSubmit(m_Queue.vkQueue, 1, &submitInfo, fence));

			VK_CALL(vkWaitForFences(m_Queue.device->ldevice(), 1, &fence, VK_TRUE, UINT64_MAX));

			if(shouldDeleteFence) {
				VK_CALLV(vkDestroyFence(m_Queue.device->ldevice(), fence, nullptr));
			}

			//VK_CALL(vkQueueWaitIdle(m_Queue.vkQueue));
		}

		free(1, &commandBuffer);
	}
}
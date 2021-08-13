#pragma once

#include "milo/graphics/api/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanCommandPool {
	private:
		VkCommandPool m_VkCommandPool;
		VulkanQueue m_Queue;
	public:
		explicit VulkanCommandPool(const VulkanQueue& queue);
		~VulkanCommandPool();
		[[nodiscard]] VkCommandPool vkCommandPool() const;
		[[nodiscard]] const VulkanQueue& queue() const;
		void allocate(VkCommandBufferLevel level, uint32_t count, VkCommandBuffer* commandBuffers);
		void free(uint32_t count, VkCommandBuffer* commandBuffers);
	};
}
#pragma once

#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/common/Common.h"

namespace milo {

	struct VulkanTask {
		VkSemaphore* waitSemaphores = nullptr;
		uint32_t waitSemaphoresCount = 0;
		VkSemaphore* signalSemaphores = nullptr;
		uint32_t signalSemaphoresCount = 0;
		VkFence fence = VK_NULL_HANDLE;
		Function<void, VkCommandBuffer> run;
		bool asynchronous = true;
	};

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
		void execute(const VulkanTask& task);
	};
}
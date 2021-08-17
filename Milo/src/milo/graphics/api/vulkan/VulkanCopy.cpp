#include "milo/graphics/api/vulkan/VulkanCopy.h"

namespace milo {

	void VulkanCopy::copy(VulkanCommandPool* commandPool, VulkanBuffer* buffer, const void* data, uint64_t size) {
		VmaMemoryUsage usage = buffer->usage();

		switch(usage) {
			case VMA_MEMORY_USAGE_GPU_ONLY:
			case VMA_MEMORY_USAGE_GPU_TO_CPU:
				copyByStagingBufferToGPU(commandPool, buffer, data, size);
				break;
			case VMA_MEMORY_USAGE_CPU_ONLY:
			case VMA_MEMORY_USAGE_CPU_TO_GPU:
			case VMA_MEMORY_USAGE_CPU_COPY:
				copyByMemoryMapping(buffer, data, size);
				break;
			default:
				throw MILO_RUNTIME_EXCEPTION(str("Unknown way to copy data to buffer with usage ") + str(usage));
		}
	}

	void VulkanCopy::copy(VulkanCommandPool* commandPool, VulkanBuffer* src, VulkanBuffer* dst, uint64_t size) {
		VulkanTask task = {};
		task.asynchronous = false;
		task.run = [&](VkCommandBuffer vkCommandBuffer) {
			VkBufferCopy copy = {};
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = size;
			VK_CALLV(vkCmdCopyBuffer(vkCommandBuffer, src->vkBuffer(), dst->vkBuffer(), 1, &copy));
		};
		commandPool->execute(task);
	}

	void VulkanCopy::copyByStagingBufferToGPU(VulkanCommandPool* commandPool, VulkanBuffer* buffer, const void* data, uint64_t size) {

		VulkanBuffer* stagingBuffer = VulkanBuffer::createStagingBuffer(data, size);

		copy(commandPool, stagingBuffer, buffer, size);

		DELETE_PTR(stagingBuffer);
	}

	void VulkanCopy::copyByMemoryMapping(VulkanBuffer* buffer, const void* data, uint64_t size) {
		VulkanMappedMemory memory = buffer->map(size);
		memcpy(memory.data, data, size);
	}
}
#pragma once

#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/vulkan/images/VulkanTexture.h"

namespace milo::VulkanCopy {

	void copy(VulkanCommandPool* commandPool, VulkanBuffer* buffer, const void* data, uint64_t size);
	void copy(VulkanCommandPool* commandPool, VulkanBuffer* src, VulkanBuffer* dst, uint64_t size = UINT64_MAX);

	void copyByStagingBufferToGPU(VulkanCommandPool* commandPool, VulkanBuffer* buffer, const void* data, uint64_t size);
	void copyByMemoryMapping(VulkanBuffer* buffer, const void* data, uint64_t size);
}
#pragma once

#include "milo/graphics/api/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/api/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/api/vulkan/images/VulkanTexture.h"

namespace milo::VulkanCopy {

	void copy(VulkanCommandPool* commandPool, VulkanBuffer* buffer, void* data, uint64_t size);
	void copy(VulkanCommandPool* commandPool, VulkanBuffer* src, VulkanBuffer* dst, uint64_t size = UINT64_MAX);

	void copyByStagingBufferToGPU(VulkanCommandPool* commandPool, VulkanBuffer* buffer, void* data, uint64_t size);
	void copyByMemoryMapping(VulkanBuffer* buffer, void* data, uint64_t size);
}
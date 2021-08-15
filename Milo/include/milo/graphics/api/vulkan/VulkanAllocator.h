#pragma once

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;
	class VulkanBuffer;
	class VulkanImage;

	class VulkanAllocator {
	private:
		VulkanContext& m_Context;
		VmaAllocator m_VmaAllocator;
	public:
		explicit VulkanAllocator(VulkanContext& context);
		~VulkanAllocator();
		[[nodiscard]] VmaAllocator vmaAllocator() const;

		void mapMemory(VmaAllocation allocation, void* data);
		void unmapMemory(VmaAllocation allocation);

		void allocateBuffer(VulkanBuffer& buffer, const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage);
		void freeBuffer(VulkanBuffer& buffer);
	};

	struct VulkanMappedMemory {
		VulkanAllocator& allocator;
		VmaAllocation_T* const allocation;
		const uint64_t size;
		int8_t* data = nullptr;

		explicit VulkanMappedMemory(VulkanAllocator& allocator, VmaAllocation allocation, uint64_t size);
		~VulkanMappedMemory();
	};
}
#pragma once

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;
	class VulkanBuffer;
	class VulkanTexture;

	class VulkanAllocator {
	private:
		VulkanContext& m_Context;
		VmaAllocator m_VmaAllocator;
	public:
		explicit VulkanAllocator(VulkanContext& context);
		~VulkanAllocator();
		[[nodiscard]] VmaAllocator vmaAllocator() const;

		void mapMemory(VmaAllocation allocation, void** data);
		void unmapMemory(VmaAllocation allocation);
		void flushMemory(VmaAllocation allocation, uint64_t offset, uint64_t size);

		void allocateBuffer(VulkanBuffer& buffer, const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage);
		void freeBuffer(VulkanBuffer& buffer);

		void allocateImage(VulkanTexture& texture, const VkImageCreateInfo& imageInfo, VmaMemoryUsage usage);
		void freeImage(VulkanTexture& texture);
	public:
		static VulkanAllocator& get();
	};

	class VulkanMappedMemory {
	private:
		VulkanAllocator& m_Allocator;
		VmaAllocation m_Allocation;
		const uint64_t m_Size;
		bool m_FlushOnUnmap = false;
		int8_t* m_Data = nullptr;
	public:
		explicit VulkanMappedMemory(VulkanAllocator& allocator, VmaAllocation allocation, uint64_t size);
		~VulkanMappedMemory();
		void* data();
		const void* data() const;
		uint64_t size() const;
		bool flushOnUnmap() const;
		void setFlushOnUnmap(bool flushOnUnmap);
		void flush(uint64_t offset = 0);
		void unmap();
		bool valid() const;
		VulkanAllocator& allocator() const;
		VmaAllocation allocation() const;

		void set(const void* srcData, uint64_t offset, uint64_t size);
		void get(void* dstData, uint64_t offset, uint64_t size) const;

		template<typename T>
		inline T* as() {
			return (T*)m_Data;
		}
	};
}
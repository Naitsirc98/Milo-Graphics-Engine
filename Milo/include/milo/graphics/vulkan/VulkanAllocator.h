#pragma once

#include "VulkanAPI.h"

namespace milo {

	class VulkanContext;
	class VulkanBuffer;
	class VulkanTexture2D;

	class VulkanAllocator {
	private:
		VulkanContext* m_Context;
		VmaAllocator m_VmaAllocator;
	public:
		explicit VulkanAllocator(VulkanContext* context);
		~VulkanAllocator();
		 VmaAllocator vmaAllocator() const;

		void mapMemory(VmaAllocation allocation, void** data);
		void unmapMemory(VmaAllocation allocation);
		void flushMemory(VmaAllocation allocation, uint64_t offset, uint64_t size);

		void allocateBuffer(const VkBufferCreateInfo& bufferInfo, const VmaAllocationCreateInfo& allocInfo, VkBuffer& vkBuffer, VmaAllocation& vmaAllocation);
		void freeBuffer(VkBuffer vkBuffer, VmaAllocation vmaAllocation);

		void allocateImage(const VkImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo, VkImage& vkImage, VmaAllocation& vmaAllocation);
		void freeImage(VkImage vkImage, VmaAllocation vmaAllocation);
	public:
		static VulkanAllocator* get();
	};
}
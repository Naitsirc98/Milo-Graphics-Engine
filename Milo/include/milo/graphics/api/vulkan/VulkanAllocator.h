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

		void allocateBuffer(VulkanBuffer& buffer, const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage);
		void freeBuffer(VulkanBuffer& buffer);
	};
}
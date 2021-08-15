#pragma once

#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/VulkanAllocator.h"

namespace milo {

	struct VulkanBufferAllocInfo {
		VkBufferCreateInfo bufferInfo = {};
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;
	};

	class VulkanBuffer {
		friend class VulkanAllocator;
	private:
		VulkanDevice& m_Device;
		ResourceHandle m_Handle = NULL;
		VkBuffer m_VkBuffer = VK_NULL_HANDLE;
		VkBufferCreateInfo m_Info = {};
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	public:
		explicit VulkanBuffer(VulkanDevice& device);
		~VulkanBuffer();

		[[nodiscard]] ResourceHandle handle() const;
		[[nodiscard]] VkBuffer vkBuffer() const;
		[[nodiscard]] const VkBufferCreateInfo& info() const;
		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] VmaAllocation& allocation();
		[[nodiscard]] uint64_t size() const;
		[[nodiscard]] bool valid() const;

		void allocate(const VulkanBufferAllocInfo& allocInfo);
		void reallocate(const VulkanBufferAllocInfo& allocInfo);
		void destroy();

		VulkanMappedMemory map(uint64_t size);

		bool operator==(const VulkanBuffer& rhs) const;
		bool operator!=(const VulkanBuffer& rhs) const;
	};
}
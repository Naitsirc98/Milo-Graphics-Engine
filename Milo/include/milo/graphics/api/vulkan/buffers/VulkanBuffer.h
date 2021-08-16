#pragma once

#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/VulkanAllocator.h"

namespace milo {

	struct VulkanBufferAllocInfo {
		VkBufferCreateInfo bufferInfo = {};
		VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;
		const void* data = nullptr;

		VulkanBufferAllocInfo();
	};

	class VulkanBuffer {
		friend class VulkanAllocator;
	private:
		VulkanDevice& m_Device;
		ResourceHandle m_Handle = NULL;
		VkBuffer m_VkBuffer = VK_NULL_HANDLE;
		VkBufferCreateInfo m_Info = {};
		VmaMemoryUsage m_Usage = VMA_MEMORY_USAGE_UNKNOWN;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
	public:
		explicit VulkanBuffer(VulkanDevice& device);
		explicit VulkanBuffer(const VulkanBuffer& other) = delete;
		~VulkanBuffer();

		[[nodiscard]] ResourceHandle handle() const;
		[[nodiscard]] VkBuffer vkBuffer() const;
		[[nodiscard]] const VkBufferCreateInfo& info() const;
		[[nodiscard]] VulkanDevice& device() const;
		[[nodiscard]] VmaMemoryUsage usage() const;
		[[nodiscard]] VmaAllocation& allocation();
		[[nodiscard]] uint64_t size() const;
		[[nodiscard]] bool valid() const;

		void allocate(const VulkanBufferAllocInfo& allocInfo);
		void reallocate(const VulkanBufferAllocInfo& allocInfo);
		void destroy();

		VulkanMappedMemory map(uint64_t size);

		VulkanBuffer& operator=(const VulkanBuffer& other) = delete;

		bool operator==(const VulkanBuffer& rhs) const;
		bool operator!=(const VulkanBuffer& rhs) const;

	public:
		static void copyToGPUBuffer(VulkanBuffer& buffer, void* data, uint64_t size);
		static void copyToCPUBuffer(VulkanBuffer& buffer, void* data, uint64_t size);
		static void copy(VulkanBuffer& dst, VulkanBuffer& src, uint64_t size);
	};
}
#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"
#include "milo/graphics/vulkan/VulkanAllocator.h"
#include "milo/graphics/vulkan/commands/VulkanCommandPool.h"
#include "milo/graphics/buffer/Buffer.h"

namespace milo {

	static const VkBufferUsageFlags VERTEX_BUFFER_USAGE_FLAGS =
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	static const VkBufferUsageFlags INDEX_BUFFER_USAGE_FLAGS =
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	static const VkBufferUsageFlags UNIFORM_BUFFER_USAGE_FLAGS =
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	static const VkBufferUsageFlags STORAGE_BUFFER_USAGE_FLAGS =
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	static const VkBufferUsageFlags STAGING_BUFFER_USAGE_FLAGS =
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	class VulkanBuffer : public Buffer {
		friend class VulkanAllocator;
	public:
		struct MemoryProperties {
			VmaMemoryUsage usage = VMA_MEMORY_USAGE_UNKNOWN;
			VkMemoryPropertyFlags propertyFlags = 0;
			VmaAllocationCreateFlags allocationFlags = 0;
		};
		struct CreateInfo {
			VulkanDevice* device = nullptr;
			VkBufferCreateInfo bufferInfo = {};
			MemoryProperties memoryProperties = {};
			CreateInfo();
		};
	private:
		VulkanDevice* m_Device;
		VkBuffer m_VkBuffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VkBufferCreateInfo m_VkBufferInfo = {};
		MemoryProperties m_MemoryProperties = {};
		void* m_MappedMemory = nullptr;
		String m_Name;
	public:
		explicit VulkanBuffer(const CreateInfo& createInfo);
		explicit VulkanBuffer(const VulkanBuffer& other) = delete;
		virtual ~VulkanBuffer() override;

		VulkanBuffer& operator=(const VulkanBuffer& other) = delete;

		VkBuffer vkBuffer() const;
		const VkBufferCreateInfo& info() const;
		VulkanDevice* device() const;
		const MemoryProperties& memoryProperties() const;
		VmaAllocation allocation();
		uint64_t size() const override;
		bool isCPUAllocated() const;

		const String& name() const;
		void setName(const String& name);

		virtual void allocate(const AllocInfo& allocInfo) override;
		virtual void update(const UpdateInfo& updateInfo) override;

		virtual MapState mapState() override;
		virtual void* mappedMemory() override;
		virtual void flush(uint64_t offset, uint64_t size) override;
		virtual void flush() override;
		virtual void* map() override;
		virtual void unmap() override;
		virtual void mapAndRun(Function<void, void*> func) override;

	public:
		static VulkanBuffer* create(Buffer::Type type);
		static VulkanBuffer* createVertexBuffer();
		static VulkanBuffer* createHostVertexBuffer();
		static VulkanBuffer* createIndexBuffer();
		static VulkanBuffer* createHostIndexBuffer();
		static VulkanBuffer* createUniformBuffer(bool alwaysMapped = false);
		static VulkanBuffer* createStorageBuffer(bool alwaysMapped = false);
		static VulkanBuffer* createStagingBuffer(uint64_t size = 0);
		static VulkanBuffer* createStagingBuffer(const void* data, uint64_t size);


		static void copy(VulkanBuffer& buffer, const void* data, uint64_t size);
		static void copy(VulkanBuffer& src, VulkanBuffer& dst, uint64_t size = UINT64_MAX);
		static void copyByStagingBufferToGPU(VulkanBuffer& buffer, const void* data, uint64_t size);
		static void copyByMemoryMapping(VulkanBuffer& buffer, const void* data, uint64_t size);
	};
}
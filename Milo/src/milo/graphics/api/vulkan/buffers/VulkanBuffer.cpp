#include "milo/graphics/api/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/api/vulkan/VulkanCopy.h"

namespace milo {

	static Atomic<ResourceHandle> g_BufferHandleProvider = 1; // TODO: graphics factory

	VulkanBuffer::VulkanBuffer(VulkanDevice& device) : m_Device(device) {
		m_Handle = g_BufferHandleProvider++;
	}

	VulkanBuffer::~VulkanBuffer() {
		destroy();
		m_Handle = NULL;
	}

	ResourceHandle VulkanBuffer::handle() const {
		return m_Handle;
	}

	VkBuffer VulkanBuffer::vkBuffer() const {
		return m_VkBuffer;
	}

	const VkBufferCreateInfo& VulkanBuffer::info() const {
		return m_Info;
	}

	VulkanDevice& VulkanBuffer::device() const {
		return m_Device;
	}

	VmaMemoryUsage VulkanBuffer::usage() const {
		return m_Usage;
	}

	VmaAllocation& VulkanBuffer::allocation() {
		return m_Allocation;
	}

	uint64_t VulkanBuffer::size() const {
		return m_Info.size;
	}

	bool VulkanBuffer::valid() const {
		return m_Allocation != VK_NULL_HANDLE;
	}

	void VulkanBuffer::allocate(const VulkanBufferAllocInfo& allocInfo) {
		if(m_Allocation != VK_NULL_HANDLE) destroy();
		m_Device.context().allocator().allocateBuffer(*this, allocInfo.bufferInfo, allocInfo.usage);
	}

	void VulkanBuffer::reallocate(const VulkanBufferAllocInfo& allocInfo) {
		destroy();
		allocate(allocInfo);
	}

	void VulkanBuffer::destroy() {
		m_Device.context().allocator().freeBuffer(*this);
	}

	VulkanMappedMemory VulkanBuffer::map(uint64_t size) {
		return VulkanMappedMemory(m_Device.context().allocator(), m_Allocation, size);
	}

	bool VulkanBuffer::operator==(const VulkanBuffer& rhs) const {
		return m_Handle == rhs.m_Handle;
	}

	bool VulkanBuffer::operator!=(const VulkanBuffer& rhs) const {
		return ! (rhs == *this);
	}


	VulkanBufferAllocInfo::VulkanBufferAllocInfo() {
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	}
}
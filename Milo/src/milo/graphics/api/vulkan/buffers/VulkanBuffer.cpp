#include "milo/graphics/api/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/api/vulkan/VulkanCopy.h"
#include "milo/graphics/Graphics.h"

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

	bool VulkanBuffer::isCPUAllocated() const {
		return m_Usage == VMA_MEMORY_USAGE_CPU_ONLY || m_Usage == VMA_MEMORY_USAGE_CPU_TO_GPU
			|| m_Usage == VMA_MEMORY_USAGE_CPU_ONLY || m_Usage == VMA_MEMORY_USAGE_CPU_COPY;
	}

	void VulkanBuffer::allocate(const VulkanBufferAllocInfo& allocInfo) {
		if(m_Allocation != VK_NULL_HANDLE) destroy();
		VulkanAllocator::get().allocateBuffer(*this, allocInfo.bufferInfo, allocInfo.usage);

		if(allocInfo.dataInfo.data != nullptr && allocInfo.dataInfo.commandPool != nullptr) {
			VulkanCopy::copy(allocInfo.dataInfo.commandPool, this, allocInfo.dataInfo.data, allocInfo.bufferInfo.size);
		}
	}

	void VulkanBuffer::reallocate(const VulkanBufferAllocInfo& allocInfo) {
		destroy();
		allocate(allocInfo);
	}

	void VulkanBuffer::destroy() {
		VulkanAllocator::get().freeBuffer(*this);
	}

	VulkanMappedMemory VulkanBuffer::map(uint64_t size) {
		return VulkanMappedMemory(VulkanAllocator::get(), m_Allocation, size == UINT64_MAX ? this->size() : size);
	}

	void VulkanBuffer::readData(void* dstData, uint64_t size) {
		if(isCPUAllocated()) {
			VulkanMappedMemory mappedMemory = this->map(size);
			mappedMemory.get(dstData, 0, size);
		} else {
			auto* stagingBuffer = createStagingBuffer(size);
			{
				VulkanCopy::copy(m_Device.transferCommandPool(), this, stagingBuffer, size);
				VulkanMappedMemory mappedMemory = stagingBuffer->map(size);
				mappedMemory.get(dstData, 0, size);
			}
			DELETE_PTR(stagingBuffer);
		}
	}

	bool VulkanBuffer::operator==(const VulkanBuffer& rhs) const {
		return m_Handle == rhs.m_Handle;
	}

	bool VulkanBuffer::operator!=(const VulkanBuffer& rhs) const {
		return ! (rhs == *this);
	}

	VulkanBufferAllocInfo::VulkanBufferAllocInfo() {
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	VulkanBuffer* VulkanBuffer::createStagingBuffer(uint64_t size) {

		auto& context = dynamic_cast<VulkanContext&>(Graphics::graphicsContext());
		VulkanDevice& device = context.device();

		auto* stagingBuffer = new VulkanBuffer(device);

		uint32_t queueFamilies[] = {device.graphicsQueue().family, device.transferQueue().family};

		VulkanBufferAllocInfo allocInfo = {};
		allocInfo.bufferInfo.size = size;
		allocInfo.bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocInfo.bufferInfo.pQueueFamilyIndices = queueFamilies;
		allocInfo.bufferInfo.queueFamilyIndexCount = 2;
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		stagingBuffer->allocate(allocInfo);

		return stagingBuffer;
	}

	VulkanBuffer* VulkanBuffer::createStagingBuffer(const void* data, uint64_t size) {

		auto& context = dynamic_cast<VulkanContext&>(Graphics::graphicsContext());
		VulkanDevice& device = context.device();

		auto* stagingBuffer = new VulkanBuffer(device);

		VulkanBufferAllocInfo allocInfo = {};
		allocInfo.bufferInfo.size = size;
		allocInfo.bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocInfo.bufferInfo.pQueueFamilyIndices = &device.transferQueue().family;
		allocInfo.bufferInfo.queueFamilyIndexCount = 1;
		allocInfo.dataInfo.data = data;
		allocInfo.dataInfo.commandPool = device.transferCommandPool();
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		stagingBuffer->allocate(allocInfo);

		return stagingBuffer;
	}
}
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/debug/VulkanDebugMessenger.h"

namespace milo {

	VulkanBuffer::CreateInfo::CreateInfo() {
		device = VulkanContext::get()->device();
		bufferInfo = mvk::BufferCreateInfo::create();
	}

	// ===

	VulkanBuffer::VulkanBuffer(const VulkanBuffer::CreateInfo& createInfo)
			: m_Device(createInfo.device), m_MemoryProperties(createInfo.memoryProperties),
			  m_VkBufferInfo(createInfo.bufferInfo) {

	}

	VulkanBuffer::~VulkanBuffer() {

		unmap();

		VulkanAllocator::get()->freeBuffer(m_VkBuffer, m_Allocation);

		m_Allocation = VK_NULL_HANDLE;
		m_VkBuffer = VK_NULL_HANDLE;
	}

	VkBuffer VulkanBuffer::vkBuffer() const {
		return m_VkBuffer;
	}

	const VkBufferCreateInfo& VulkanBuffer::info() const {
		return m_VkBufferInfo;
	}

	VulkanDevice* VulkanBuffer::device() const {
		return m_Device;
	}

	VmaAllocation VulkanBuffer::allocation() {
		return m_Allocation;
	}

	uint64_t VulkanBuffer::size() const {
		return m_VkBufferInfo.size;
	}

	const VulkanBuffer::MemoryProperties& VulkanBuffer::memoryProperties() const {
		return m_MemoryProperties;
	}

	bool VulkanBuffer::isCPUAllocated() const {
		const VmaMemoryUsage usage = m_MemoryProperties.usage;
		return usage == VMA_MEMORY_USAGE_CPU_ONLY || usage == VMA_MEMORY_USAGE_CPU_TO_GPU || usage == VMA_MEMORY_USAGE_CPU_COPY;
	}

	const String& VulkanBuffer::name() const {
		return m_Name;
	}

	void VulkanBuffer::setName(const String& name) {
		VulkanDebugMessenger::setName(m_VkBuffer, name.c_str());
		m_Name = name;
	}

	void VulkanBuffer::allocate(const AllocInfo& allocInfo) {

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = m_MemoryProperties.usage;
		vmaAllocInfo.requiredFlags = m_MemoryProperties.propertyFlags;
		vmaAllocInfo.flags = m_MemoryProperties.allocationFlags;

		m_VkBufferInfo.size = allocInfo.size;

		VulkanAllocator::get()->allocateBuffer(m_VkBufferInfo, vmaAllocInfo, m_VkBuffer, m_Allocation);

		if(allocInfo.data != nullptr) {

			UpdateInfo updateInfo = {};
			updateInfo.offset = 0;
			updateInfo.size = allocInfo.size;
			updateInfo.data = allocInfo.data;

			update(updateInfo);
		}
	}

	void VulkanBuffer::update(const UpdateInfo& updateInfo) {
#ifdef _DEBUG
		if(updateInfo.offset + updateInfo.size > size()) throw MILO_RUNTIME_EXCEPTION("updateInfo.offset + updateInfo.size > size()");
		if(updateInfo.data == nullptr) return;
#endif

		copy(*this, updateInfo.data, updateInfo.size);
	}

	Buffer::MapState VulkanBuffer::mapState() {
		if(!isCPUAllocated()) return MapState::Unavailable;

		if((m_MemoryProperties.allocationFlags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0) {
			return MapState::AlwaysMapped;
		}

		return m_MappedMemory == nullptr ? MapState::Unmapped : MapState::Mapped;
	}

	void* VulkanBuffer::mappedMemory() {
		return m_MappedMemory;
	}

	void VulkanBuffer::flush(uint64_t offset, uint64_t size) {
		VK_CALL(vmaFlushAllocation(VulkanAllocator::get()->vmaAllocator(), m_Allocation, offset, size));
	}

	void VulkanBuffer::flush() {
		flush(0, size());
	}

	void* VulkanBuffer::map() {
		switch(mapState()) {
			case MapState::Unmapped:
			VK_CALL(vmaMapMemory(VulkanAllocator::get()->vmaAllocator(), m_Allocation, &m_MappedMemory));
			case MapState::Mapped:
			case MapState::AlwaysMapped:
				return m_MappedMemory;
			case MapState::Unavailable:
			default:
				throw MILO_RUNTIME_EXCEPTION("This buffer cannot be mapped");
		}
	}

	void VulkanBuffer::unmap() {
		switch(mapState()) {
			case MapState::Mapped:
			VK_CALLV(vmaUnmapMemory(VulkanAllocator::get()->vmaAllocator(), m_Allocation));
				m_MappedMemory = nullptr;
			case MapState::Unmapped:
			case MapState::AlwaysMapped:
			case MapState::Unavailable:
				break;
		}
	}

	void VulkanBuffer::mapAndRun(Function<void, void*> func) {
		bool shouldUnmap = m_MappedMemory == nullptr;
		if(shouldUnmap) map();
		func(m_MappedMemory);
		if(shouldUnmap) unmap();
	}

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


	VulkanBuffer* VulkanBuffer::create(Buffer::Type type) {
		switch(type) {
			case Type::Vertex: return createVertexBuffer();
			case Type::Index: return createIndexBuffer();
			case Type::Uniform: return createUniformBuffer();
			case Type::Storage: return createStorageBuffer();
			default:
				throw MILO_RUNTIME_EXCEPTION("Unsupported type of buffer");
		}
	}

	VulkanBuffer* VulkanBuffer::createVertexBuffer() {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(VERTEX_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createHostVertexBuffer() {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(VERTEX_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createIndexBuffer() {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(INDEX_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createHostIndexBuffer() {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(INDEX_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createUniformBuffer(bool alwaysMapped) {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(UNIFORM_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		if(alwaysMapped)
			createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createStorageBuffer(bool alwaysMapped) {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(STORAGE_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		if(alwaysMapped)
			createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		return new VulkanBuffer(createInfo);
	}

	VulkanBuffer* VulkanBuffer::createStagingBuffer(uint64_t size) {
		return createStagingBuffer(nullptr, 0);
	}

	VulkanBuffer* VulkanBuffer::createStagingBuffer(const void* data, uint64_t size) {
		CreateInfo createInfo = {};
		createInfo.bufferInfo = mvk::BufferCreateInfo::create(STAGING_BUFFER_USAGE_FLAGS);
		createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VulkanBuffer* stagingBuffer = new VulkanBuffer(createInfo);

		if(data != nullptr && size > 0) {
			AllocInfo allocInfo = {};
			allocInfo.size = size;
			allocInfo.data = data;
			stagingBuffer->allocate(allocInfo);
		}

		return stagingBuffer;
	}

	void VulkanBuffer::copy(VulkanBuffer& buffer, const void* data, uint64_t size) {

		VmaMemoryUsage usage = buffer.memoryProperties().usage;

		switch(usage) {
			case VMA_MEMORY_USAGE_GPU_ONLY:
			case VMA_MEMORY_USAGE_GPU_TO_CPU:
				copyByStagingBufferToGPU(buffer, data, size);
				break;
			case VMA_MEMORY_USAGE_CPU_ONLY:
			case VMA_MEMORY_USAGE_CPU_TO_GPU:
			case VMA_MEMORY_USAGE_CPU_COPY:
				copyByMemoryMapping(buffer, data, size);
				break;
			default:
				throw MILO_RUNTIME_EXCEPTION(str("Unknown way to copy data to buffer with usage ") + str(usage));
		}
	}

	void VulkanBuffer::copy(VulkanBuffer& src, VulkanBuffer& dst, uint64_t size) {

		VulkanCommandPool* commandPool = src.device()->transferCommandPool();

		VulkanTask task = {};
		task.asynchronous = false;
		task.run = [&](VkCommandBuffer vkCommandBuffer) {
			VkBufferCopy copy = {};
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = size;
			VK_CALLV(vkCmdCopyBuffer(vkCommandBuffer, src.vkBuffer(), dst.vkBuffer(), 1, &copy));
		};

		commandPool->execute(task);
	}

	void VulkanBuffer::copyByStagingBufferToGPU(VulkanBuffer& buffer, const void* data, uint64_t size) {
		VulkanBuffer* stagingBuffer = VulkanBuffer::createStagingBuffer(data, size);
		copy(*stagingBuffer, buffer, size);
		DELETE_PTR(stagingBuffer);
	}

	void VulkanBuffer::copyByMemoryMapping(VulkanBuffer& buffer, const void* data, uint64_t size) {
		buffer.mapAndRun([&](void* mappedMemory) {memcpy(mappedMemory, data, size);});
	}
}
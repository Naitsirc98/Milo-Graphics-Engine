#include "milo/graphics/api/vulkan/VulkanAllocator.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/Graphics.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace milo {

	VulkanAllocator& VulkanAllocator::get() {
		auto& context = dynamic_cast<VulkanContext&>(Graphics::graphicsContext());
		return context.allocator();
	}

	VulkanAllocator::VulkanAllocator(VulkanContext& context) : m_Context(context) {

		VmaAllocatorCreateInfo createInfo = {};
		createInfo.instance = m_Context.vkInstance();
		createInfo.physicalDevice = m_Context.device().pdevice();
		createInfo.device = m_Context.device().ldevice();
		createInfo.frameInUseCount = m_Context.swapchain().imageCount();
		createInfo.vulkanApiVersion = VK_MAKE_VERSION(1, 2, 0);

		VK_CALL(vmaCreateAllocator(&createInfo, &m_VmaAllocator));
	}

	VulkanAllocator::~VulkanAllocator() {
		VK_CALLV(vmaDestroyAllocator(m_VmaAllocator));
		m_VmaAllocator = nullptr;
	}

	VmaAllocator VulkanAllocator::vmaAllocator() const {
		return m_VmaAllocator;
	}

	void VulkanAllocator::mapMemory(VmaAllocation allocation, void** data) {
		VK_CALL(vmaMapMemory(m_VmaAllocator, allocation, data));
	}

	void VulkanAllocator::unmapMemory(VmaAllocation allocation) {
		VK_CALLV(vmaUnmapMemory(m_VmaAllocator, allocation));
	}

	void VulkanAllocator::flushMemory(VmaAllocation allocation, uint64_t offset, uint64_t size) {
		VK_CALL(vmaFlushAllocation(m_VmaAllocator, allocation, offset, size));
	}

	void VulkanAllocator::allocateBuffer(VulkanBuffer& buffer, const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage) {
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = usage;
		VK_CALL(vmaCreateBuffer(m_VmaAllocator, &bufferInfo, &allocInfo, &buffer.m_VkBuffer, &buffer.m_Allocation, nullptr));
		buffer.m_Info = bufferInfo;
		buffer.m_Usage = usage;
	}

	void VulkanAllocator::freeBuffer(VulkanBuffer& buffer) {
		if(buffer.vkBuffer() == VK_NULL_HANDLE || buffer.allocation()  == VK_NULL_HANDLE) return;
		VK_CALLV(vmaDestroyBuffer(m_VmaAllocator, buffer.m_VkBuffer, buffer.m_Allocation));
		buffer.m_VkBuffer = VK_NULL_HANDLE;
		buffer.m_Allocation = VK_NULL_HANDLE;
	}

	void VulkanAllocator::allocateImage(VulkanTexture& texture, const VkImageCreateInfo& imageInfo, VmaMemoryUsage usage) {
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = usage;
		VK_CALL(vmaCreateImage(m_VmaAllocator, &imageInfo, &allocInfo, &texture.m_VkImage, &texture.m_Allocation, nullptr));
		texture.m_VkImageInfo = imageInfo;
		texture.m_ImageLayout = imageInfo.initialLayout;
	}

	void VulkanAllocator::freeImage(VulkanTexture& texture) {
		if(texture.vkImage() == VK_NULL_HANDLE || texture.allocation() == VK_NULL_HANDLE) return;
		VK_CALLV(vmaDestroyImage(m_VmaAllocator, texture.m_VkImage, texture.m_Allocation));
		texture.m_VkImage = VK_NULL_HANDLE;
		texture.m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		texture.m_Allocation = VK_NULL_HANDLE;
	}


	//========

#ifdef _DEBUG
#define CHECK_MAPPED if(!this->valid()) throw MILO_RUNTIME_EXCEPTION("VulkanMappedMemory was already unmapped!");
#else
#define CHECK_MAPPED
#endif

	VulkanMappedMemory::VulkanMappedMemory(VulkanAllocator& allocator, VmaAllocation allocation, uint64_t size)
		: m_Allocator(allocator), m_Allocation(allocation), m_Size(size) {

		allocator.mapMemory(allocation, (void**)&m_Data);
	}

	VulkanMappedMemory::~VulkanMappedMemory() {
		unmap();
	}

	void* VulkanMappedMemory::data() {
		return m_Data;
	}

	const void* VulkanMappedMemory::data() const {
		return m_Data;
	}

	uint64_t VulkanMappedMemory::size() const {
		return m_Size;
	}

	bool VulkanMappedMemory::flushOnUnmap() const {
		return m_FlushOnUnmap;
	}

	void VulkanMappedMemory::setFlushOnUnmap(bool flushOnUnmap) {
		m_FlushOnUnmap = flushOnUnmap;
	}

	void VulkanMappedMemory::flush(uint64_t offset) {
		m_Allocator.flushMemory(m_Allocation, offset, m_Size);
	}

	void VulkanMappedMemory::unmap() {
		if(!valid()) return;
		if(m_FlushOnUnmap) flush();
		m_Allocator.unmapMemory(m_Allocation);
		m_Data = nullptr;
	}

	bool VulkanMappedMemory::valid() const {
		return m_Data != nullptr;
	}

	VulkanAllocator& VulkanMappedMemory::allocator() const {
		return m_Allocator;
	}

	VmaAllocation VulkanMappedMemory::allocation() const {
		return m_Allocation;
	}

	void VulkanMappedMemory::set(const void* srcData, uint64_t offset, uint64_t size) {
		CHECK_MAPPED
		memcpy(m_Data + offset, srcData, size);
	}

	void VulkanMappedMemory::get(void* dstData, uint64_t offset, uint64_t size) const {
		CHECK_MAPPED
		memcpy(dstData, m_Data + offset, size);
	}
}
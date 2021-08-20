#pragma once

#include "VulkanBuffer.h"

namespace milo {

	template<typename T>
	class VulkanUniformBuffer {
	private:
		VulkanBuffer* m_Buffer;
		uint32_t m_MinAlignment;
		uint32_t m_ElementSize;
	public:
		VulkanUniformBuffer(VulkanDevice& device) : m_Buffer(new VulkanBuffer(device)) {
			m_MinAlignment = m_Buffer->device().pDeviceInfo().uniformBufferAlignment();
			m_ElementSize = roundUp2((uint32_t)sizeof(T), m_MinAlignment);
		}

		~VulkanUniformBuffer() {
			DELETE_PTR(m_Buffer);
		}

		void allocate(uint32_t numElements) {

			uint32_t queueFamilies = {device().graphicsQueue().family};

			VulkanBufferAllocInfo allocInfo = {};
			allocInfo.bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			allocInfo.bufferInfo.size = m_ElementSize * numElements;
			allocInfo.bufferInfo.pQueueFamilyIndices = &queueFamilies;
			allocInfo.bufferInfo.queueFamilyIndexCount = 1;
			allocInfo.bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

			m_Buffer->allocate(allocInfo);
		}

		void update(uint32_t elementIndex, const T& value) {
			VulkanMappedMemory mappedMemory = map();
			mappedMemory.set(&value, elementIndex * m_ElementSize, sizeof(T));
		}

		inline VulkanMappedMemory map(uint64_t size = UINT64_MAX) {
			return m_Buffer->map(size);
		}

		inline VulkanBuffer& buffer() const {
			return *m_Buffer;
		}

		inline VulkanDevice& device() const {
			return m_Buffer->device();
		}

		inline VkBuffer vkBuffer() const {
			return m_Buffer->vkBuffer();
		}

		inline uint32_t minAlignment() const {
			return m_MinAlignment;
		}

		inline uint32_t elementSize() const {
			return m_ElementSize;
		}

		inline uint64_t size() const {
			return buffer().size();
		}

		inline uint32_t numElements() const {
			return size() / elementSize();
		}
	};

}
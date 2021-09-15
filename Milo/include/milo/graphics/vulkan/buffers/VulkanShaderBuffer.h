#pragma once

#include "VulkanBuffer.h"

namespace milo {

	template<typename T>
	class VulkanShaderBuffer {
	protected:
		VulkanBuffer* m_Buffer;
		uint32_t m_MinAlignment;
		uint32_t m_ElementSize;
	public:
		VulkanShaderBuffer() = default;

		virtual ~VulkanShaderBuffer() {
			DELETE_PTR(m_Buffer);
		}

		void allocate(uint32_t numElements) {
			Buffer::AllocInfo allocInfo = {};
			allocInfo.size = numElements * m_ElementSize;
			m_Buffer->allocate(allocInfo);
		}

		void update(uint32_t elementIndex, const T& value) {
			byte_t* mappedMemory = (byte_t*)m_Buffer->map();
			memcpy(mappedMemory + elementIndex * m_ElementSize, &value, sizeof(T));
		}

		inline VulkanBuffer& buffer() const {
			return *m_Buffer;
		}

		inline VulkanDevice* device() const {
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

	template<typename T>
	class VulkanUniformBuffer : public VulkanShaderBuffer<T> {
	public:
		VulkanUniformBuffer() {
			m_Buffer = VulkanBuffer::createUniformBuffer();
			m_MinAlignment = m_Buffer->device()->info().uniformBufferAlignment();
			m_ElementSize = roundUp2((uint32_t)sizeof(T), m_MinAlignment);
		}
	public:
		static VulkanUniformBuffer<T>* create() {
			return new VulkanUniformBuffer<T>();
		}
	};

	template<typename T>
	class VulkanStorageBuffer : public VulkanShaderBuffer<T> {
	public:
		VulkanStorageBuffer() {
			m_Buffer = VulkanBuffer::createStorageBuffer(false);
			m_MinAlignment = m_Buffer->device()->info().storageMinAlignment();
			m_ElementSize = roundUp2((uint32_t)sizeof(T), m_MinAlignment);
		}
	public:
		static VulkanStorageBuffer<T>* create() {
			return new VulkanUniformBuffer<T>();
		}
	};

}
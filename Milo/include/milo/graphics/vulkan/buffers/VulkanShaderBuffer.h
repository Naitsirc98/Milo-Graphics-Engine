#pragma once

#include "VulkanBuffer.h"

namespace milo {

	template<typename T>
	class VulkanShaderBuffer : public VulkanBuffer {
	protected:
		uint32_t m_MinAlignment{0};
		uint32_t m_ElementSize{0};
	public:
		explicit VulkanShaderBuffer(const VulkanBuffer::CreateInfo& createInfo) : VulkanBuffer(createInfo) {
			if(createInfo.bufferInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
				m_MinAlignment = device()->info().uniformBufferAlignment();
			} else if(createInfo.bufferInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
				m_MinAlignment = device()->info().storageBufferAlignment();
			} else {
				throw MILO_RUNTIME_EXCEPTION("Unsupported usage for VulkanShaderBuffer");
			}

			m_ElementSize = roundUp2((uint32_t)sizeof(T), m_MinAlignment);
		}

		virtual ~VulkanShaderBuffer() override = default;

		void allocate(uint32_t numElements) {
			Buffer::AllocInfo allocInfo = {};
			allocInfo.size = numElements * m_ElementSize;
			VulkanBuffer::allocate(allocInfo);
		}

		void update(uint32_t elementIndex, const T& value) {
			byte_t* mappedMemory = (byte_t*)map();
			memcpy(mappedMemory + elementIndex * m_ElementSize, &value, sizeof(T));
		}

		inline uint32_t minAlignment() const {
			return m_MinAlignment;
		}

		inline uint32_t elementSize() const {
			return m_ElementSize;
		}

		inline uint32_t numElements() const {
			return size() / elementSize();
		}
	};

	template<typename T>
	class VulkanUniformBuffer : public VulkanShaderBuffer<T> {
	public:
		explicit VulkanUniformBuffer(const VulkanBuffer::CreateInfo& createInfo) : VulkanShaderBuffer<T>(createInfo) {
		}
	public:
		static VulkanUniformBuffer<T>* create(bool alwaysMapped = false) {
			VulkanBuffer::CreateInfo createInfo = {};
			createInfo.bufferInfo = mvk::BufferCreateInfo::create(UNIFORM_BUFFER_USAGE_FLAGS);
			createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			if(alwaysMapped) createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			return new VulkanUniformBuffer<T>(createInfo);
		}
	};

	template<typename T>
	class VulkanStorageBuffer : public VulkanShaderBuffer<T> {
	public:
		explicit VulkanStorageBuffer(const VulkanBuffer::CreateInfo& createInfo) : VulkanShaderBuffer<T>(createInfo) {
		}
	public:
		static VulkanStorageBuffer<T>* create(bool alwaysMapped = false) {
			VulkanBuffer::CreateInfo createInfo = {};
			createInfo.bufferInfo = mvk::BufferCreateInfo::create(STORAGE_BUFFER_USAGE_FLAGS);
			createInfo.memoryProperties.propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			createInfo.memoryProperties.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			if(alwaysMapped)
				createInfo.memoryProperties.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			return new VulkanStorageBuffer<T>(createInfo);
		}
	};

}
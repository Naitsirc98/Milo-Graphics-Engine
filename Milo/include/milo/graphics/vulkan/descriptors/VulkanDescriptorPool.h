#pragma once

#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanDescriptorPool {
	public:
		struct CreateInfo {
			VkDescriptorSetLayout layout = VK_NULL_HANDLE;
			size_t capacity = 0;
			ArrayList<VkDescriptorPoolSize> poolSizes;
			size_t initialSize = 0;
			VkDescriptorPoolCreateFlags flags = 0;
		};
	private:
		VulkanDevice* m_Device;
		VkDescriptorSetLayout m_VkDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_VkDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet* m_DescriptorSets = nullptr;
		VkDescriptorPoolCreateFlags m_Flags = 0;
		size_t m_Capacity;
		size_t m_Size = 0;
	public:
		VulkanDescriptorPool(VulkanDevice* device, const VulkanDescriptorPool::CreateInfo& createInfo);
		~VulkanDescriptorPool();
		VulkanDevice* device() const;
		VkDescriptorPool vkDescriptorPool() const;
		size_t size() const;
		size_t capacity() const;
		void allocate(size_t numSets = SIZE_MAX, const Function<void, size_t, VkDescriptorSet>& updateDescriptorSetFunction = nullptr);
		void free(size_t numSets = SIZE_MAX);
		VkDescriptorSet* descriptorSets() const;
		VkDescriptorSet get(size_t index) const;
		VkDescriptorSet operator[](size_t index) const;
	};
}
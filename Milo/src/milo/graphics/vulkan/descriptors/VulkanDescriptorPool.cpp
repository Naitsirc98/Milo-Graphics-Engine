#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"

namespace milo {

	VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice& device, const VulkanDescriptorPool::CreateInfo& createInfo)
	: m_Device(device), m_VkDescriptorSetLayout(createInfo.layout), m_Capacity(createInfo.capacity), m_Flags(createInfo.flags) {

		m_DescriptorSets = new VkDescriptorSet[m_Capacity];

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = createInfo.capacity;
		poolInfo.pPoolSizes = createInfo.poolSizes.data();
		poolInfo.poolSizeCount = createInfo.poolSizes.size();
		poolInfo.flags = createInfo.flags;

		VK_CALL(vkCreateDescriptorPool(m_Device.ldevice(), &poolInfo, nullptr, &m_VkDescriptorPool));

		if(createInfo.initialSize > 0) allocate(createInfo.initialSize);
	}

	VulkanDescriptorPool::~VulkanDescriptorPool() {
		DELETE_ARRAY(m_DescriptorSets);
		VK_CALLV(vkDestroyDescriptorPool(m_Device.ldevice(), m_VkDescriptorPool, nullptr));
		m_VkDescriptorPool = VK_NULL_HANDLE;
		m_Capacity = 0;
	}

	VulkanDevice& VulkanDescriptorPool::device() const {
		return m_Device;
	}

	VkDescriptorPool VulkanDescriptorPool::vkDescriptorPool() const {
		return m_VkDescriptorPool;
	}

	size_t VulkanDescriptorPool::size() const {
		return m_Size;
	}

	size_t VulkanDescriptorPool::capacity() const {
		return m_Capacity;
	}

	void VulkanDescriptorPool::allocate(size_t numSets, const Function<void, size_t, VkDescriptorSet>& updateDescriptorSetFunction) {
		if(numSets == SIZE_MAX) numSets = m_Capacity - m_Size;
#ifdef _DEBUG
		if(m_Size + numSets > m_Capacity) throw MILO_RUNTIME_EXCEPTION(str("NumSets >= capacity: ") + str(numSets) + " >= " + str(m_Capacity));
#endif

		ArrayList<VkDescriptorSetLayout> layouts(numSets, m_VkDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_VkDescriptorPool;
		allocInfo.pSetLayouts = layouts.data();
		allocInfo.descriptorSetCount = numSets;

		VK_CALL(vkAllocateDescriptorSets(m_Device.ldevice(), &allocInfo, &m_DescriptorSets[m_Size]));

		if(updateDescriptorSetFunction) {
			for(size_t i = 0;i < numSets;++i) {
				size_t index = i + m_Size;
				updateDescriptorSetFunction(index, m_DescriptorSets[index]);
			}
		}

		m_Size += numSets;
	}

	void VulkanDescriptorPool::free(size_t numSets) {
#ifdef _DEBUG
		if((m_Flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0)
			throw MILO_RUNTIME_EXCEPTION("Attempting to free individual descriptor set allocations, but this pool was not created with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT");

		if(numSets > m_Size) throw MILO_RUNTIME_EXCEPTION(str("NumSets > size: ") + str(numSets) + " > " + str(m_Size));
#endif

		VK_CALLV(vkFreeDescriptorSets(m_Device.ldevice(), m_VkDescriptorPool, numSets, &m_DescriptorSets[m_Size - numSets]));
		memset(&m_DescriptorSets[m_Size - numSets], NULL, numSets * sizeof(VkDescriptorSet));
		m_Size -= numSets;
	}

	VkDescriptorSet VulkanDescriptorPool::get(size_t index) const {
#ifdef _DEBUG
		if(index >= m_Size) throw MILO_RUNTIME_EXCEPTION(str("Index for descriptor set is out of bounds: ") + str(index) + " >= " + str(m_Size));
#endif
		return m_DescriptorSets[index];
	}

	VkDescriptorSet VulkanDescriptorPool::operator[](size_t index) const {
		return get(index);
	}
}
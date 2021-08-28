#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanMaterialResourcePool::VulkanMaterialResourcePool() {
		m_Device = VulkanContext::get()->device();
		createUniformBuffer();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
		m_MaterialIndices.reserve(m_MaxMaterialCount);
	}

	VulkanMaterialResourcePool::~VulkanMaterialResourcePool() {
		DELETE_PTR(m_DescriptorPool);
		DELETE_PTR(m_UniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
	}

	inline static VkImageView getImageView(Texture2D* texture) {
		if(texture == nullptr) return VK_NULL_HANDLE;
		return dynamic_cast<VulkanTexture2D*>(texture)->vkImageView();
	}

	inline static VkSampler getSampler(Texture2D* texture) {
		if(texture == nullptr) return VK_NULL_HANDLE;
		return dynamic_cast<VulkanTexture2D*>(texture)->vkSampler();
	}

	void VulkanMaterialResourcePool::allocateMaterialResources(Material* material) {

		uint32_t index;

		if(!m_FreeIndices.empty()) {
			index = m_FreeIndices.front();
			m_FreeIndices.pop_front();
		} else {
			index = m_MaterialCount++;
		}

		m_MaterialIndices[material->name()] = index;

		m_UniformBuffer->update(index, material->data());

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(index);

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer->vkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Material::Data);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = getImageView(material->albedoMap());
		imageInfo.sampler = getSampler(material->albedoMap());

		// TODO: more textures

		VkWriteDescriptorSet writeDescriptors[2]{};

		using namespace mvk::WriteDescriptorSet;

		writeDescriptors[0] = createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);
		writeDescriptors[1] = createCombineImageSamplerWrite(1, descriptorSet, Material::TEXTURE_COUNT, &imageInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 2, writeDescriptors, 0, nullptr));
	}

	void VulkanMaterialResourcePool::freeMaterialResources(Material* material) {
		uint32_t index = m_MaterialIndices[material->name()];
		m_MaterialIndices.erase(material->name());
		m_FreeIndices.push_back(index);
	}

	VkDescriptorSet VulkanMaterialResourcePool::descriptorSetOf(Material* material, uint32_t& dynamicOffset) {
		uint32_t index = m_MaterialIndices[material->name()];
		dynamicOffset = index * m_UniformBuffer->elementSize();
		return m_DescriptorPool->get(index);
	}

	VkDescriptorSetLayout VulkanMaterialResourcePool::materialDescriptorSetLayout() const {
		return m_DescriptorSetLayout;
	}

	void VulkanMaterialResourcePool::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<Material::Data>();
		m_MaxMaterialCount = std::floor(m_Device->info().uniformBufferMaxSize() / m_UniformBuffer->elementSize());
		m_UniformBuffer->allocate(m_MaxMaterialCount);
		Log::debug("Supporting {} different materials", m_MaxMaterialCount);
	}

	void VulkanMaterialResourcePool::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};
		// Material Data
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		// Material textures
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].descriptorCount = Material::TEXTURE_COUNT;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanMaterialResourcePool::createDescriptorPool() {

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.capacity = m_MaxMaterialCount;
		createInfo.layout = m_DescriptorSetLayout;

		Array<VkDescriptorPoolSize, 2> poolSizes{};

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[0].descriptorCount = m_MaxMaterialCount;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = m_MaxMaterialCount * Material::TEXTURE_COUNT;

		createInfo.poolSizes.push_back(poolSizes[0]);
		createInfo.poolSizes.push_back(poolSizes[1]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanMaterialResourcePool::createDescriptorSets() {
		m_DescriptorPool->allocate(m_MaxMaterialCount);
	}
}
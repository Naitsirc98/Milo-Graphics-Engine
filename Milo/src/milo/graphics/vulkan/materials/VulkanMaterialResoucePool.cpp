#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanMaterialResourcePool::VulkanMaterialResourcePool() {
		m_Device = VulkanContext::get()->device();
		createUniformBuffer();
		createDescriptorSetLayoutAndPool();
		createDescriptorSets();
		m_MaterialIndices.reserve(m_MaxMaterialCount);
	}

	VulkanMaterialResourcePool::~VulkanMaterialResourcePool() {
		DELETE_PTR(m_DescriptorPool);
		DELETE_PTR(m_UniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
	}

	inline static VkImageView getImageView(Ref<Texture2D> texture) {
		if(texture == nullptr) return VK_NULL_HANDLE;
		return dynamic_cast<VulkanTexture2D*>(texture.get())->vkImageView();
	}

	inline static VkSampler getSampler(Ref<Texture2D> texture) {
		if(texture == nullptr) return VK_NULL_HANDLE;
		return dynamic_cast<VulkanTexture2D*>(texture.get())->vkSampler();
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

		VkDescriptorImageInfo albedoInfo{};
		albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoInfo.imageView = getImageView(material->albedoMap());
		albedoInfo.sampler = getSampler(material->albedoMap());

		VkDescriptorImageInfo emissiveInfo{};
		emissiveInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		emissiveInfo.imageView = getImageView(material->emissiveMap());
		emissiveInfo.sampler = getSampler(material->emissiveMap());

		VkDescriptorImageInfo normalInfo{};
		normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalInfo.imageView = getImageView(material->normalMap());
		normalInfo.sampler = getSampler(material->normalMap());

		VkDescriptorImageInfo metallicInfo{};
		metallicInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		metallicInfo.imageView = getImageView(material->metallicMap());
		metallicInfo.sampler = getSampler(material->metallicMap());

		VkDescriptorImageInfo roughnessInfo{};
		roughnessInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		roughnessInfo.imageView = getImageView(material->roughnessMap());
		roughnessInfo.sampler = getSampler(material->roughnessMap());

		VkDescriptorImageInfo occlusionInfo{};
		occlusionInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		occlusionInfo.imageView = getImageView(material->occlusionMap());
		occlusionInfo.sampler = getSampler(material->occlusionMap());

		using namespace mvk::WriteDescriptorSet;

		VkWriteDescriptorSet writeDescriptors[] = {
				createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo),
				createCombineImageSamplerWrite(1, descriptorSet, 1, &albedoInfo),
				createCombineImageSamplerWrite(2, descriptorSet, 1, &emissiveInfo),
				createCombineImageSamplerWrite(3, descriptorSet, 1, &normalInfo),
				createCombineImageSamplerWrite(4, descriptorSet, 1, &metallicInfo),
				createCombineImageSamplerWrite(5, descriptorSet, 1, &roughnessInfo),
				createCombineImageSamplerWrite(6, descriptorSet, 1, &occlusionInfo),
		};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1 + Material::TEXTURE_COUNT, writeDescriptors, 0, nullptr));
	}

	void VulkanMaterialResourcePool::freeMaterialResources(Material* material) {
		uint32_t index = m_MaterialIndices[material->name()];
		m_MaterialIndices.erase(material->name());
		m_FreeIndices.push_back(index);
	}

	VkDescriptorSet VulkanMaterialResourcePool::descriptorSetOf(Material* material, uint32_t& dynamicOffset) const {
		uint32_t index = m_MaterialIndices.at(material->name());
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

	void VulkanMaterialResourcePool::createDescriptorSetLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		createInfo.numSets = m_MaxMaterialCount;
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_DescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_DescriptorPool = mvk::DescriptorSet::Pool::create(m_DescriptorSetLayout, createInfo);
	}

	void VulkanMaterialResourcePool::createDescriptorSets() {
		m_DescriptorPool->allocate(m_MaxMaterialCount);
	}
}
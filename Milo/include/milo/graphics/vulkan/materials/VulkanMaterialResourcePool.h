#pragma once

#include "milo/assets/materials/MaterialResourcePool.h"
#include "milo/graphics/vulkan/buffers/VulkanShaderBuffer.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"

namespace milo {

	// TODO: make this scalable

	class VulkanMaterialResourcePool : public MaterialResourcePool {
		friend class MaterialManager;
		friend class MaterialResourcePool;
	private:
		VulkanDevice* m_Device{nullptr};
		VulkanUniformBuffer<Material::Data>* m_UniformBuffer{nullptr};
		uint64_t m_MaxMaterialCount{0};
		VkDescriptorSetLayout m_DescriptorSetLayout{VK_NULL_HANDLE};
		VulkanDescriptorPool* m_DescriptorPool{nullptr};
		Queue<uint32_t> m_FreeIndices;
		uint32_t m_MaterialCount{0};
		HashMap<String, uint32_t> m_MaterialIndices;
	private:
		VulkanMaterialResourcePool();
		~VulkanMaterialResourcePool();
	public:
		void allocateMaterialResources(Material* material) override;
		void updateMaterial(Material* material) override;
		void freeMaterialResources(Material* material) override;
		VkDescriptorSet descriptorSetOf(Material* material, uint32_t& dynamicOffset) const;
		VkDescriptorSetLayout materialDescriptorSetLayout() const;
	private:
		void createUniformBuffer();
		void createDescriptorSetLayoutAndPool();
		void createDescriptorSets();
		void updateTextures(Material* material);
	};

}
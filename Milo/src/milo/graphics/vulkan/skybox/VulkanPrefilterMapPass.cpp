#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	VulkanPrefilterMapPass::VulkanPrefilterMapPass(VulkanDevice* device) : m_Device(device) {

		createDescriptorSetLayout();
		createDescriptorPool();
		createPipelineLayout();
		createComputePipeline();
	}

	VulkanPrefilterMapPass::~VulkanPrefilterMapPass() {

		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_ComputePipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
	}

	void VulkanPrefilterMapPass::execute(const VulkanSkyboxPassExecuteInfo& execInfo) {

		VK_CALLV(vkCmdBindPipeline(execInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

		updateDescriptorSet(execInfo);

		VK_CALLV(vkCmdDispatch(execInfo.commandBuffer, 32, 32, 32)); // TODO
	}

	void VulkanPrefilterMapPass::updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo) {

		using namespace mvk::WriteDescriptorSet;

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);

		VkDescriptorImageInfo equirectangularTextureInfo{};
		equirectangularTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		equirectangularTextureInfo.imageView = execInfo.equirectangularTexture->vkImageView();
		equirectangularTextureInfo.sampler = execInfo.equirectangularTexture->vkSampler();

		VkDescriptorImageInfo environmentMapInfo{};
		environmentMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		environmentMapInfo.imageView = execInfo.environmentMap->vkImageView();
		environmentMapInfo.sampler = execInfo.environmentMap->vkSampler();

		VkWriteDescriptorSet equirectangularTextureWrite = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(0, descriptorSet, 1, &equirectangularTextureInfo);
		VkWriteDescriptorSet environmentMapWrite = mvk::WriteDescriptorSet::createStorageImageWrite(1, descriptorSet, 1, &environmentMapInfo);

		VkWriteDescriptorSet writeDescriptors[] = {equirectangularTextureWrite, environmentMapWrite};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 2, writeDescriptors, 0, nullptr));
	}

	void VulkanPrefilterMapPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};

		// Equirectangular image
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		// Environment map
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bindings[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanPrefilterMapPass::createDescriptorPool() {

		Array<VkDescriptorPoolSize, 2> poolSizes{};
		// Equirectangular texture
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 1;

		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[1].descriptorCount = 1;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = 1;
		createInfo.poolSizes.push_back(poolSizes[0]);
		createInfo.poolSizes.push_back(poolSizes[1]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanPrefilterMapPass::createPipelineLayout() {

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = &m_DescriptorSetLayout;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanPrefilterMapPass::createComputePipeline() {

		Shader* shader = Assets::shaders().load(Files::resource("shaders/skybox/equirectangular_to_cubemap.comp"));
		auto* vulkanShader = dynamic_cast<VulkanShader*>(shader);

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pCode = (uint32_t*)vulkanShader->bytecode();
		shaderModuleCreateInfo.codeSize = vulkanShader->bytecodeLength();

		VkShaderModule shaderModule;
		VK_CALL(vkCreateShaderModule(m_Device->logical(), &shaderModuleCreateInfo, nullptr, &shaderModule));

		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.module = shaderModule;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.pName = "main";

		VkComputePipelineCreateInfo createInfo{};
		createInfo.layout = m_PipelineLayout;
		createInfo.stage = shaderStage;

		VK_CALL(vkCreateComputePipelines(m_Device->logical(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_ComputePipeline));
	}
}
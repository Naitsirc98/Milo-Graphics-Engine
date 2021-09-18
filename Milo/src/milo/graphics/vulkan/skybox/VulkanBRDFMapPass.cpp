#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanBRDFMapPass::VulkanBRDFMapPass(VulkanDevice* device) : m_Device(device) {

		createDescriptorSetLayout();
		createDescriptorPool();
		createPipelineLayout();
		createComputePipeline();
	}

	VulkanBRDFMapPass::~VulkanBRDFMapPass() {

		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_ComputePipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
	}

	void VulkanBRDFMapPass::execute(const VulkanSkyboxPassExecuteInfo& execInfo) {

		VkCommandBuffer commandBuffer = execInfo.commandBuffer;
		VulkanTexture2D* brdfMap = execInfo.brdfMap;

		uint32_t mapSize = execInfo.loadInfo->brdfSize;

		if(brdfMap->vkImageView() == VK_NULL_HANDLE) {

			Texture2D::AllocInfo allocInfo{};
			allocInfo.width = mapSize;
			allocInfo.height = mapSize;
			allocInfo.format = PixelFormat::RG16F;
			allocInfo.mipLevels = 1;

			brdfMap->allocate(allocInfo);

			VkSamplerCreateInfo sampler{};
			sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler.magFilter = VK_FILTER_LINEAR;
			sampler.minFilter = VK_FILTER_LINEAR;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.minLod = 0.0f;
			sampler.maxLod = 1.0f;
			sampler.maxAnisotropy = 1.0f;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

			brdfMap->vkSampler(VulkanContext::get()->samplerMap()->get(sampler));
		}

		VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

		brdfMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
						   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		updateDescriptorSet(execInfo);

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout,
										 0, 1, &descriptorSet, 0, nullptr));

		VK_CALLV(vkCmdDispatch(commandBuffer, mapSize / 32, mapSize / 32, 1));

		brdfMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						   VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	void VulkanBRDFMapPass::updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo) {

		using namespace mvk::WriteDescriptorSet;

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);

		VkDescriptorImageInfo envMapInfo{};
		envMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		envMapInfo.imageView = execInfo.environmentMap->vkImageView();
		envMapInfo.sampler = execInfo.environmentMap->vkSampler();

		VkDescriptorImageInfo brdfMapInfo{};
		brdfMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		brdfMapInfo.imageView = execInfo.brdfMap->vkImageView();
		brdfMapInfo.sampler = execInfo.brdfMap->vkSampler();

		VkWriteDescriptorSet equirectangularTextureWrite = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(0, descriptorSet, 1, &envMapInfo);
		VkWriteDescriptorSet environmentMapWrite = mvk::WriteDescriptorSet::createStorageImageWrite(1, descriptorSet, 1, &brdfMapInfo);

		VkWriteDescriptorSet writeDescriptors[] = {equirectangularTextureWrite, environmentMapWrite};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 2, writeDescriptors, 0, nullptr));
	}

	void VulkanBRDFMapPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};

		// Equirectangular image
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[0].descriptorCount = 1;
		// Environment map
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bindings[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanBRDFMapPass::createDescriptorPool() {

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

		m_DescriptorPool->allocate(1);
	}

	void VulkanBRDFMapPass::createPipelineLayout() {

		VkPushConstantRange pushConstants{};
		pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(uint32_t);

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = &m_DescriptorSetLayout;
		createInfo.pPushConstantRanges = &pushConstants;
		createInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanBRDFMapPass::createComputePipeline() {

		Shader* shader = Assets::shaders().load(Files::resource("shaders/skybox/brdf.comp"));
		auto* vulkanShader = dynamic_cast<VulkanShader*>(shader);

		VkShaderModuleCreateInfo shaderModuleCreateInfo{};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.pCode = (uint32_t*)vulkanShader->bytecode();
		shaderModuleCreateInfo.codeSize = vulkanShader->bytecodeLength();

		VkShaderModule shaderModule;
		VK_CALL(vkCreateShaderModule(m_Device->logical(), &shaderModuleCreateInfo, nullptr, &shaderModule));

		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.module = shaderModule;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.pName = "main";

		VkComputePipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.layout = m_PipelineLayout;
		createInfo.stage = shaderStage;

		VK_CALL(vkCreateComputePipelines(m_Device->logical(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &m_ComputePipeline));
	}
}
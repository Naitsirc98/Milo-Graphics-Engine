#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	VulkanPrefilterMapPass::VulkanPrefilterMapPass(VulkanDevice* device) : m_Device(device) {

		createDescriptorSetLayoutAndPool();
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

		VkCommandBuffer commandBuffer = execInfo.commandBuffer;
		VulkanCubemap* prefilterMap = execInfo.prefilterMap;

		uint32_t mapSize = execInfo.loadInfo->prefilterMapSize;

		if(prefilterMap->vkImageView() == VK_NULL_HANDLE) {
			Cubemap::AllocInfo allocInfo{};
			allocInfo.width = mapSize;
			allocInfo.height = mapSize;
			allocInfo.format = PixelFormat::RGBA32F;
			allocInfo.mipLevels = 4;

			prefilterMap->allocate(allocInfo);
			prefilterMap->generateMipmaps();

			VkSamplerCreateInfo sampler{};
			sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler.magFilter = VK_FILTER_LINEAR;
			sampler.minFilter = VK_FILTER_LINEAR;
			sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			sampler.minLod = 0.0f;
			sampler.maxLod = static_cast<float>(allocInfo.mipLevels);
			sampler.maxAnisotropy = 1.0f;
			sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

			prefilterMap->vkSampler(VulkanContext::get()->samplerMap()->get(sampler));
		}

		VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

		prefilterMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		updateDescriptorSet(execInfo);

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout,
										 0, 1, &descriptorSet, 0, nullptr));

		PushConstants pushConstants{};
		pushConstants.roughness = 0.5f;
		pushConstants.level = 0;

		VK_CALLV(vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants), &pushConstants));

		VK_CALLV(vkCmdDispatch(commandBuffer, mapSize / 32, mapSize / 32, 6));

		prefilterMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	void VulkanPrefilterMapPass::updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo) {

		using namespace mvk::WriteDescriptorSet;

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);

		VkDescriptorImageInfo equirectangularTextureInfo{};
		equirectangularTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		equirectangularTextureInfo.imageView = execInfo.environmentMap->vkImageView();
		equirectangularTextureInfo.sampler = execInfo.environmentMap->vkSampler();

		VkDescriptorImageInfo environmentMapInfo{};
		environmentMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		environmentMapInfo.imageView = execInfo.prefilterMap->vkImageView();
		environmentMapInfo.sampler = execInfo.prefilterMap->vkSampler();

		VkWriteDescriptorSet equirectangularTextureWrite = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(0, descriptorSet, 1, &equirectangularTextureInfo);
		VkWriteDescriptorSet environmentMapWrite = mvk::WriteDescriptorSet::createStorageImageWrite(1, descriptorSet, 1, &environmentMapInfo);

		VkWriteDescriptorSet writeDescriptors[] = {equirectangularTextureWrite, environmentMapWrite};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 2, writeDescriptors, 0, nullptr));
	}

	void VulkanPrefilterMapPass::createDescriptorSetLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.numSets = 1;
		createInfo.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

		m_DescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_DescriptorPool = mvk::DescriptorSet::Pool::create(m_DescriptorSetLayout, createInfo);

		m_DescriptorPool->allocate(1);
	}


	void VulkanPrefilterMapPass::createPipelineLayout() {

		VkPushConstantRange pushConstants{};
		pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = &m_DescriptorSetLayout;
		createInfo.pPushConstantRanges = &pushConstants;
		createInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanPrefilterMapPass::createComputePipeline() {

		Shader* shader = Assets::shaders().load(Files::resource("shaders/skybox/prefilter.comp"));
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
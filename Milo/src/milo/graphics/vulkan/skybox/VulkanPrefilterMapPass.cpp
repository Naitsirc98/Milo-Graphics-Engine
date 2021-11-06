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

		uint32_t mipLevels = static_cast<uint32_t>(roundf(execInfo.loadInfo->maxLOD) + 1);

		if(prefilterMap->vkImageView() == VK_NULL_HANDLE) {
			Cubemap::AllocInfo allocInfo{};
			allocInfo.width = mapSize;
			allocInfo.height = mapSize;
			allocInfo.format = PixelFormat::RGBA16F;
			allocInfo.mipLevels = mipLevels;

			prefilterMap->allocate(allocInfo);

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

			prefilterMap->generateMipmaps();

			prefilterMap->createMipImageViews();
		}

		VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

		prefilterMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		updateDescriptorSet(execInfo);

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout,
										 0, 1, &descriptorSet, 0, nullptr));

		int32_t envMapResolution = (int32_t)execInfo.environmentMap->width();

		for(int32_t i = 0;i < mipLevels;++i) {

			uint32_t mipLevelSize = static_cast<uint32_t>(mapSize * powf(0.5f, i));

			PushConstants pushConstants{};
			pushConstants.roughness = 0;//(float)i / (float)(mipLevels - 1);
			pushConstants.envMapResolution = envMapResolution;
			pushConstants.mipLevel = i;
			pushConstants.numSamples = 1024;
			VK_CALLV(vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
										0, sizeof(PushConstants), &pushConstants));

			VK_CALLV(vkCmdDispatch(commandBuffer, mipLevelSize / 32, mipLevelSize / 32, 6));
		}

		prefilterMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	void VulkanPrefilterMapPass::updateDescriptorSet(const VulkanSkyboxPassExecuteInfo& execInfo) {

		using namespace mvk::WriteDescriptorSet;

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);

		VkWriteDescriptorSet writeDescriptors[6]{};

		VkDescriptorImageInfo envMapInfo{};
		envMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		envMapInfo.imageView = execInfo.environmentMap->vkImageView();
		envMapInfo.sampler = execInfo.environmentMap->vkSampler();

		writeDescriptors[0] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(0, descriptorSet, 1, &envMapInfo);

		VkDescriptorImageInfo prefilterInfo[5]{};

		for(uint32_t i = 0;i < 5;++i) {
			prefilterInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			prefilterInfo[i].imageView = execInfo.prefilterMap->getMipImageView(i);
			prefilterInfo[i].sampler = execInfo.prefilterMap->vkSampler();

			writeDescriptors[i + 1] = mvk::WriteDescriptorSet::createStorageImageWrite(1, descriptorSet, 1, &prefilterInfo[i]);
			writeDescriptors[i + 1].dstArrayElement = i;
		}

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 6, writeDescriptors, 0, nullptr));
	}

	void VulkanPrefilterMapPass::createDescriptorSetLayoutAndPool() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};
		// Environment map
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// Prefilter map
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].descriptorCount = 5;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pBindings = bindings.data();
		layoutInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &layoutInfo, nullptr, &m_DescriptorSetLayout));

		VkDescriptorPoolSize poolSizes[2] = {
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
				{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 5}
		};

		VulkanDescriptorPool::CreateInfo poolInfo{};
		poolInfo.layout = m_DescriptorSetLayout;
		poolInfo.capacity = 1;
		poolInfo.initialSize = 1;
		poolInfo.poolSizes.push_back(poolSizes[0]);
		poolInfo.poolSizes.push_back(poolSizes[1]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, poolInfo);
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
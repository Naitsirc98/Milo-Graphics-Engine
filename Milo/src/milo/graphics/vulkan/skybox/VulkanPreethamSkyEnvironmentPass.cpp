#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/graphics/vulkan/skybox/VulkanSkyboxFactory.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	VulkanPreethamSkyEnvironmentPass::VulkanPreethamSkyEnvironmentPass(VulkanDevice* device) {
		m_Device = device;
		createDescriptorSetLayout();
		createDescriptorPool();
		createPipelineLayout();
		createComputePipeline();
	}

	VulkanPreethamSkyEnvironmentPass::~VulkanPreethamSkyEnvironmentPass() {
		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_ComputePipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));
		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
	}

	void VulkanPreethamSkyEnvironmentPass::execute(const VulkanSkyboxPassExecuteInfo& execInfo) {

		VkCommandBuffer commandBuffer = execInfo.commandBuffer;
		VulkanCubemap* environmentMap = execInfo.environmentMap;

		uint32_t mapSize = execInfo.loadInfo->environmentMapSize;

		if(environmentMap->vkImageView() == VK_NULL_HANDLE) {
			Cubemap::AllocInfo allocInfo{};
			allocInfo.width = mapSize;
			allocInfo.height = mapSize;
			allocInfo.format = PixelFormat::RGBA16F;
			allocInfo.mipLevels = 4;

			environmentMap->allocate(allocInfo);
			environmentMap->generateMipmaps();

			VkSamplerCreateInfo samplerInfo = mvk::SamplerCreateInfo::create();
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.compareOp = VK_COMPARE_OP_NEVER;

			VkSampler sampler = VulkanContext::get()->samplerMap()->get(samplerInfo);

			environmentMap->vkSampler(sampler);
		}

		VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

		environmentMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL,
								  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		const float pushConstants[] = {execInfo.turbidity, execInfo.azimuth, execInfo.inclination};
		VK_CALLV(vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 3 * sizeof(float), pushConstants));

		updateDescriptorSet(environmentMap);

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_PipelineLayout,
										 0, 1, &descriptorSet, 0, nullptr));

		VK_CALLV(vkCmdDispatch(commandBuffer, mapSize / 32, mapSize / 32, 6));

		environmentMap->setLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
								  VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	}

	void VulkanPreethamSkyEnvironmentPass::update(VkCommandBuffer commandBuffer, VulkanCubemap* environmentMap,
												  float turbidity, float azimuth, float inclination) {


	}

	void VulkanPreethamSkyEnvironmentPass::updateDescriptorSet(VulkanCubemap* environmentMap) {

		using namespace mvk::WriteDescriptorSet;

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);

		VkDescriptorImageInfo environmentMapInfo{};
		environmentMapInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		environmentMapInfo.imageView = environmentMap->vkImageView();
		environmentMapInfo.sampler = environmentMap->vkSampler();

		VkWriteDescriptorSet environmentMapWrite = mvk::WriteDescriptorSet::createStorageImageWrite(0, descriptorSet, 1, &environmentMapInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &environmentMapWrite, 0, nullptr));
	}

	void VulkanPreethamSkyEnvironmentPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 1> bindings{};

		// Environment map
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		bindings[0].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanPreethamSkyEnvironmentPass::createDescriptorPool() {

		VkDescriptorPoolSize poolSizes{};
		// Environment Map
		poolSizes.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes.descriptorCount = 1;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = 1;
		createInfo.initialSize = 1;
		createInfo.poolSizes.push_back(poolSizes);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanPreethamSkyEnvironmentPass::createPipelineLayout() {

		VkPushConstantRange pushConstants{};
		pushConstants.offset = 0;
		pushConstants.size = 3 * sizeof(float);
		pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.setLayoutCount = 1;
		createInfo.pSetLayouts = &m_DescriptorSetLayout;
		createInfo.pPushConstantRanges = &pushConstants;
		createInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanPreethamSkyEnvironmentPass::createComputePipeline() {

		Shader* shader = Assets::shaders().load(Files::resource("shaders/skybox/preetham_sky.comp"));
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
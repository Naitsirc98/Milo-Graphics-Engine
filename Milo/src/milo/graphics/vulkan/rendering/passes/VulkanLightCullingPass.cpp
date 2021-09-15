#include "milo/graphics/vulkan/rendering/passes/VulkanLightCullingPass.h"
#include "milo/graphics/rendering/passes/PreDepthRenderPass.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"

namespace milo {

	VulkanLightCullingPass::VulkanLightCullingPass() {
		m_Device = VulkanContext::get()->device();
		createCameraUniformBuffer();
		createPointLightsUniformBuffer();
		createVisibleLightIndicesStorageBuffer();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
		createComputePipeline();
	}

	VulkanLightCullingPass::~VulkanLightCullingPass() {

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_ComputePipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_PointLightsUniformBuffer);
		DELETE_PTR(m_VisibleLightsStorageBuffer);
	}

	bool VulkanLightCullingPass::shouldCompile(Scene* scene) const {
		Array<Ref<Framebuffer>, MAX_SWAPCHAIN_IMAGE_COUNT> depthFramebuffers = {
				WorldRenderer::get().resources().getFramebuffer(PreDepthRenderPass::getFramebufferHandle(0)),
				WorldRenderer::get().resources().getFramebuffer(PreDepthRenderPass::getFramebufferHandle(1)),
				WorldRenderer::get().resources().getFramebuffer(PreDepthRenderPass::getFramebufferHandle(2))
		};
		if(m_LastFramebuffers != depthFramebuffers) {
			m_LastFramebuffers = depthFramebuffers;
			return true;
		}
		return false;
	}

	void VulkanLightCullingPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {
		updatePreDepthMap();
	}

	void VulkanLightCullingPass::execute(Scene* scene) {




	}

	void VulkanLightCullingPass::updatePreDepthMap() {

		for(uint32_t i = 0;i < m_LastFramebuffers.size();++i) {

			auto* depthMap = (VulkanTexture2D*)m_LastFramebuffers[i]->depthAttachments()[0];

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = depthMap->vkImageView();
			imageInfo.sampler = depthMap->vkSampler();

			VkDescriptorSet descriptorSet = m_DescriptorPool->get(i);
			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(3, descriptorSet, 1, &imageInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		}
	}

	void VulkanLightCullingPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 3> bindings{};
		// Camera uniform buffer
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		// PointLights uniform buffer
		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		// Visible lights indices storage buffer
		bindings[2].binding = 2;
		bindings[2].descriptorCount = 1;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanLightCullingPass::createDescriptorPool() {

		VkDescriptorPoolSize poolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_SWAPCHAIN_IMAGE_COUNT},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_SWAPCHAIN_IMAGE_COUNT},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_SWAPCHAIN_IMAGE_COUNT}
		};

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.poolSizes.push_back(poolSizes[0]);
		createInfo.poolSizes.push_back(poolSizes[1]);
		createInfo.poolSizes.push_back(poolSizes[2]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanLightCullingPass::createCameraUniformBuffer() {
		m_CameraUniformBuffer = new VulkanUniformBuffer<CameraUniformBuffer>();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanLightCullingPass::createPointLightsUniformBuffer() {
		m_PointLightsUniformBuffer = new VulkanUniformBuffer<PointLightsUniformBuffer>();
		m_PointLightsUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanLightCullingPass::createVisibleLightIndicesStorageBuffer() {

		m_VisibleLightsStorageBuffer = VulkanBuffer::createStorageBuffer(false);

		VulkanBuffer::AllocInfo allocInfo{};
		allocInfo.size = 8192 * sizeof(uint32_t);

		m_VisibleLightsStorageBuffer->allocate(allocInfo);
	}

	void VulkanLightCullingPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [this](uint32_t imageIndex, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraBufferInfo = {};
			cameraBufferInfo.buffer = m_CameraUniformBuffer->vkBuffer();
			cameraBufferInfo.offset = 0;
			cameraBufferInfo.range = sizeof(CameraUniformBuffer);

			VkDescriptorBufferInfo pointLightsBufferInfo = {};
			pointLightsBufferInfo.buffer = m_PointLightsUniformBuffer->vkBuffer();
			pointLightsBufferInfo.offset = 0;
			pointLightsBufferInfo.range = sizeof(PointLightsUniformBuffer);

			VkDescriptorBufferInfo visibleIndicesBufferInfo = {};
			pointLightsBufferInfo.buffer = m_VisibleLightsStorageBuffer->vkBuffer();
			pointLightsBufferInfo.offset = 0;
			pointLightsBufferInfo.range = 8192 * sizeof(uint32_t);

			VkWriteDescriptorSet writeDescriptorSets[3] = {
					mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &cameraBufferInfo),
					mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &pointLightsBufferInfo),
					mvk::WriteDescriptorSet::createStorageBufferWrite(0, descriptorSet, 1, &visibleIndicesBufferInfo)
			};

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 3, writeDescriptorSets, 0, nullptr));
		});
	}

	void VulkanLightCullingPass::createComputePipeline() {

		VkPushConstantRange pushConstants{};
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);
		pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkPipelineLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		layoutCreateInfo.pPushConstantRanges = &pushConstants;
		layoutCreateInfo.pushConstantRangeCount = 1;
		layoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
		layoutCreateInfo.setLayoutCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &layoutCreateInfo, nullptr, &m_PipelineLayout));

		const VulkanShader* shader = (const VulkanShader*)Assets::shaders().load("resources/shaders/light_culling/light_culling.comp");

		VkShaderModule shaderModule = VK_NULL_HANDLE;

		{
			VkShaderModuleCreateInfo shaderModuleCreateInfo{};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = shader->bytecodeLength();
			shaderModuleCreateInfo.pCode = (uint32_t*)shader->bytecode();

			VK_CALL(vkCreateShaderModule(m_Device->logical(), &shaderModuleCreateInfo, nullptr, &shaderModule));
		}

		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStage.module = shaderModule;
		shaderStage.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.stage = shaderStage;

		VK_CALL(vkCreateComputePipelines(m_Device->logical(), nullptr, 1, &pipelineInfo, nullptr, &m_ComputePipeline));

		VK_CALLV(vkDestroyShaderModule(m_Device->logical(), shaderModule, nullptr));
	}
}

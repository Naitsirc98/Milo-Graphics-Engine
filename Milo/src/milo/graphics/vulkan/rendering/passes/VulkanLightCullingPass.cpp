#include "milo/graphics/vulkan/rendering/passes/VulkanLightCullingPass.h"
#include "milo/graphics/rendering/passes/PreDepthRenderPass.h"
#include "milo/graphics/vulkan/shaders/VulkanShader.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

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
		createSemaphores();
		createCommandBuffers();
	}

	VulkanLightCullingPass::~VulkanLightCullingPass() {

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyPipeline(m_Device->logical(), m_ComputePipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(m_Device->logical(), m_PipelineLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_PointLightsUniformBuffer);

		mvk::Semaphore::destroy(m_SignalSemaphores.size(), m_SignalSemaphores.data());

		m_Device->computeCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	bool VulkanLightCullingPass::shouldCompile(Scene* scene) const {
		return false;
	}

	void VulkanLightCullingPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {
	}

	void VulkanLightCullingPass::execute(Scene* scene) {

		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* computeQueue = m_Device->computeQueue();
		VulkanQueue* graphicsQueue = m_Device->graphicsQueue();

		buildCommandBuffer(imageIndex, commandBuffer, scene);

		VkPipelineStageFlags waitDstStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = graphicsQueue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = graphicsQueue->waitSemaphores().size();
		submitInfo.pWaitDstStageMask = &waitDstStageFlags;
		submitInfo.pSignalSemaphores = &m_SignalSemaphores[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;

		computeQueue->submit(submitInfo, VK_NULL_HANDLE);

		graphicsQueue->waitSemaphores().clear();
		graphicsQueue->waitSemaphores().push_back(m_SignalSemaphores[imageIndex]);
	}

	void VulkanLightCullingPass::buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		updateUniforms(imageIndex, scene);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputePipeline));

			VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);
			uint32_t dynamicOffsets[] = {
					(uint32_t) (imageIndex * m_CameraUniformBuffer->elementSize()),
					(uint32_t) (imageIndex * m_PointLightsUniformBuffer->elementSize()),
					(uint32_t) (imageIndex * sizeof(VisibleLightsBuffer))};

			VK_CALLV(vkCmdBindDescriptorSets(commandBuffer,
											 VK_PIPELINE_BIND_POINT_COMPUTE,
											 m_PipelineLayout,
											 0, 1, &descriptorSet,
											 3, dynamicOffsets));

			PushConstants pushConstants{};
			pushConstants.screenSize = scene->viewportSize();

			VK_CALLV(vkCmdPushConstants(commandBuffer, m_PipelineLayout,
										VK_SHADER_STAGE_COMPUTE_BIT,
										0, sizeof(PushConstants),
										&pushConstants));

			const Size viewport = scene->viewportSize();
			const uint32_t workGroupsX = (viewport.width + (viewport.width % TILE_SIZE)) / TILE_SIZE;
			const uint32_t workGroupsY = (viewport.height + (viewport.height % TILE_SIZE)) / TILE_SIZE;

			VK_CALLV(vkCmdDispatch(commandBuffer, workGroupsX, workGroupsY, 1));
		}
		VK_CALLV(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanLightCullingPass::updateUniforms(uint32_t imageIndex, Scene* scene) {

		{
			const auto& camera = WorldRenderer::get().camera();
			CameraUniformBuffer cameraData{};
			cameraData.projectionMatrix = camera.proj;
			cameraData.viewMatrix = camera.view;
			cameraData.viewProjectionMatrix = camera.projView;
			m_CameraUniformBuffer->update(imageIndex, cameraData);
		}

		{
			const auto& lights = WorldRenderer::get().lights();
			PointLightsUniformBuffer pointLightsData{};
			memcpy(pointLightsData.pointLights, lights.pointLights.data(), lights.pointLights.size() * sizeof(PointLight));
			pointLightsData.pointLightsCount = lights.pointLights.size();
			m_PointLightsUniformBuffer->update(imageIndex, pointLightsData);
		}

		auto framebuffer = WorldRenderer::get().resources().getFramebuffer(PreDepthRenderPass::getFramebufferHandle(imageIndex));
		auto* depthMap = (VulkanTexture2D*)framebuffer->colorAttachments()[0];
		depthMap->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = depthMap->vkImageView();
		imageInfo.sampler = depthMap->vkSampler();

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);

		VkWriteDescriptorSet writeDescriptorSets[] = {
				mvk::WriteDescriptorSet::createCombineImageSamplerWrite(3, descriptorSet, 1, &imageInfo)
		};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, writeDescriptorSets, 0, nullptr));
	}

	void VulkanLightCullingPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 4> bindings{};
		// Camera uniform buffer
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		// PointLights uniform buffer
		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		// Visible lights indices storage buffer
		bindings[2].binding = 2;
		bindings[2].descriptorCount = 1;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		// Depth map texture
		bindings[3].binding = 3;
		bindings[3].descriptorCount = 1;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanLightCullingPass::createDescriptorPool() {

		VkDescriptorPoolSize poolSizes[] = {
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_SWAPCHAIN_IMAGE_COUNT},
				{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_SWAPCHAIN_IMAGE_COUNT},
				{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, MAX_SWAPCHAIN_IMAGE_COUNT},
				{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_SWAPCHAIN_IMAGE_COUNT},
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
		m_CameraUniformBuffer = VulkanUniformBuffer<CameraUniformBuffer>::create();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanLightCullingPass::createPointLightsUniformBuffer() {
		m_PointLightsUniformBuffer = VulkanUniformBuffer<PointLightsUniformBuffer>::create();
		m_PointLightsUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanLightCullingPass::createVisibleLightIndicesStorageBuffer() {

		m_VisibleLightsStorageBuffer = Ref<VulkanStorageBuffer<VisibleLightsBuffer>>(
				VulkanStorageBuffer<VisibleLightsBuffer>::create());

		m_VisibleLightsStorageBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);

		WorldRenderer::get().resources().putBuffer(LightCullingPass::getVisibleLightsBufferHandle(), m_VisibleLightsStorageBuffer);
	}

	void VulkanLightCullingPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [this](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraBufferInfo = {};
			cameraBufferInfo.buffer = m_CameraUniformBuffer->vkBuffer();
			cameraBufferInfo.offset = 0;
			cameraBufferInfo.range = sizeof(CameraUniformBuffer);

			VkDescriptorBufferInfo pointLightsBufferInfo = {};
			pointLightsBufferInfo.buffer = m_PointLightsUniformBuffer->vkBuffer();
			pointLightsBufferInfo.offset = 0;
			pointLightsBufferInfo.range = sizeof(PointLightsUniformBuffer);

			VkDescriptorBufferInfo visibleIndicesBufferInfo = {};
			visibleIndicesBufferInfo.buffer = m_VisibleLightsStorageBuffer->vkBuffer();
			visibleIndicesBufferInfo.offset = 0;
			visibleIndicesBufferInfo.range = sizeof(VisibleLightsBuffer);

			VkWriteDescriptorSet writeDescriptorSets[3] = {
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &cameraBufferInfo),
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(1, descriptorSet, 1, &pointLightsBufferInfo),
					mvk::WriteDescriptorSet::createDynamicStorageBufferWrite(2, descriptorSet, 1, &visibleIndicesBufferInfo)
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
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCreateInfo.pPushConstantRanges = &pushConstants;
		layoutCreateInfo.pushConstantRangeCount = 1;
		layoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
		layoutCreateInfo.setLayoutCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &layoutCreateInfo, nullptr, &m_PipelineLayout));

		const VulkanShader* shader = (const VulkanShader*)Assets::shaders().load("resources/shaders/culling/light_culling.comp");

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

	void VulkanLightCullingPass::createCommandBuffers() {
		m_Device->computeCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	void VulkanLightCullingPass::createSemaphores() {
		mvk::Semaphore::create(m_SignalSemaphores.size(), m_SignalSemaphores.data());
	}
}

#include <milo/graphics/vulkan/buffers/VulkanMeshBuffers.h>
#include "milo/graphics/vulkan/rendering/passes/VulkanPBRForwardRenderPass.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"

namespace milo {

	VulkanPBRForwardRenderPass::VulkanPBRForwardRenderPass() {

		m_Device = VulkanContext::get()->device();

		createRenderPass();

		createCameraUniformBuffer();
		createRendererDataUniformBuffer();
		createSceneDataUniformBuffer();

		createSceneDescriptorLayoutAndPool();
		createSceneDescriptorSets();

		createSkyboxDescriptorLayoutAndPool();
		createSkyboxDescriptorSets();

		createShadowsDescriptorLayoutAndPool();
		createShadowsDescriptorSets();

		createGraphicsPipeline();

		createSemaphores();

		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	VulkanPBRForwardRenderPass::~VulkanPBRForwardRenderPass() {

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyRenderPass(device, m_RenderPass, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_RenderUniformBuffer);
		DELETE_PTR(m_SceneUniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_SceneDescriptorSetLayout, nullptr));
		DELETE_PTR(m_SceneDescriptorPool);

		DELETE_PTR(m_GraphicsPipeline);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALLV(vkDestroySemaphore(device, m_SignalSemaphores[i], nullptr));
		}

		m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	bool VulkanPBRForwardRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}

	void VulkanPBRForwardRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {
	}

	void VulkanPBRForwardRenderPass::execute(Scene* scene) {

		const uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* queue = m_Device->graphicsQueue();

		buildCommandBuffer(imageIndex, commandBuffer, scene);

		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pSignalSemaphores = &m_SignalSemaphores[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitDstStageMask = &waitStageMask;

		VkFence fence = queue->lastFence();
		queue->submit(submitInfo, VK_NULL_HANDLE);
		queue->setFence(fence);
	}

	void VulkanPBRForwardRenderPass::buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		mvk::CommandBuffer::BeginGraphicsRenderPassInfo beginInfo{};
		beginInfo.renderPass = m_RenderPass;
		beginInfo.graphicsPipeline = m_GraphicsPipeline->vkPipeline();

		mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);
		{
			renderScene(imageIndex, commandBuffer, scene);
		}
		mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
	}

	inline void VulkanPBRForwardRenderPass::renderScene(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		auto& materialResources = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		Mesh* lastMesh = nullptr;
		Material* lastMaterial = nullptr;

		updateCameraUniformData(imageIndex);
		updateRendererDataUniformData(imageIndex);
		updateSceneDataUniformData(imageIndex);

		updateSkyboxUniformData(imageIndex, scene->skyboxView()->skybox);

		// TODO: Shadows

		for(DrawCommand drawCommand : WorldRenderer::get().drawCommands()) {

			const Matrix4& transform = drawCommand.transform;
			Mesh* mesh = drawCommand.mesh;
			Material* material = drawCommand.material;

			if(lastMaterial != material) {
				bindMaterial(commandBuffer, materialResources, material);
				lastMaterial = material;
			}

			if(lastMesh != mesh) {
				bindMesh(commandBuffer, mesh);
				lastMesh = mesh;
			}

			pushConstants(commandBuffer, transform);

			drawMesh(commandBuffer, mesh);
		}
	}

	void VulkanPBRForwardRenderPass::drawMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const {
		if(mesh->indices().empty()) {
			VK_CALLV(vkCmdDraw(commandBuffer, mesh->vertices().size(), 1, 0, 0));
		} else {
			VK_CALLV(vkCmdDrawIndexed(commandBuffer, mesh->indices().size(), 1, 0, 0, 0));
		}
	}

	void VulkanPBRForwardRenderPass::pushConstants(VkCommandBuffer commandBuffer, const Matrix4& transform) const {
		PushConstants pushConstants = {transform};
		VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
									0, sizeof(PushConstants), &pushConstants));
	}

	void VulkanPBRForwardRenderPass::bindMesh(VkCommandBuffer commandBuffer, const Mesh* mesh) const {
		VulkanMeshBuffers* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());

		VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
		VkDeviceSize offsets[] = {0};
		VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

		if(!mesh->indices().empty()) {
			VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
		}
	}

	void VulkanPBRForwardRenderPass::bindMaterial(VkCommandBuffer commandBuffer,
												  const VulkanMaterialResourcePool& materialResources,
												  Material* material) const {
		uint32_t dynamicOffset;

		VkDescriptorSet materialDescriptorSet = materialResources.descriptorSetOf(material, dynamicOffset);
		VkDescriptorSet descriptorSets[] = {materialDescriptorSet};

		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										 m_GraphicsPipeline->pipelineLayout(),
										 0, 1, descriptorSets, 1, &dynamicOffset));
	}

	inline void VulkanPBRForwardRenderPass::updateCameraUniformData(uint32_t imageIndex) {

		const auto& camera = WorldRenderer::get().camera();

		CameraData cameraData{};
		cameraData.viewMatrix = camera.view;
		cameraData.projViewMatrix = camera.projView;

		m_CameraUniformBuffer->update(imageIndex, cameraData);
	}

	void VulkanPBRForwardRenderPass::updateRendererDataUniformData(uint32_t imageIndex) {

		RendererData data{};
		data.u_CascadeFading = true;
		data.u_CascadeSplits = Vector4(0); // TODO
		data.u_CascadeTransitionFade = 0.1f; // TODO
		//data.u_LightMatrix = {}; // TODO
		data.u_LightSize = 0; // TODO
		data.u_MaxShadowDistance = 0; // TODO
		data.u_ShadowFade = 0.1f; // TODO
		data.u_SoftShadows = true;
		data.u_ShowLightComplexity = false;
		data.u_TilesCountX = 0; // TODO

		m_RenderUniformBuffer->update(imageIndex, data);
	}

	void VulkanPBRForwardRenderPass::updateSceneDataUniformData(uint32_t imageIndex) {

		const auto& camera = WorldRenderer::get().camera();
		const auto& lights = WorldRenderer::get().lights();

		SceneData data{};
		data.u_CameraPosition = Vector3(camera.view[3]);
		data.u_DirectionalLights = lights.dirLight;
		data.u_PointLightsCount = lights.pointLights.size();
		memcpy(data.u_pointLights, lights.pointLights.data(), lights.pointLights.size() * sizeof(PointLight));
		data.u_EnvironmentMapIntensity = 1;

		m_SceneUniformBuffer->update(imageIndex, data);
	}

	void VulkanPBRForwardRenderPass::updateSkyboxUniformData(uint32_t imageIndex, const Skybox* skybox) {

		bool present = skybox != nullptr;
		auto* defaultCubemap = dynamic_cast<VulkanCubemap*>(Assets::textures().blackCubemap().get());
		auto* defaultTexture = dynamic_cast<VulkanTexture2D*>(Assets::textures().blackTexture().get());

		VkDescriptorSet descriptorSet = m_SceneDescriptorPool->get(imageIndex);

		VkDescriptorImageInfo irradianceMapInfo{};
		irradianceMapInfo.imageView = present ? mvk::getImageView(skybox->irradianceMap()) : defaultCubemap->vkImageView();
		irradianceMapInfo.sampler = present ? mvk::getSampler(skybox->irradianceMap()) : defaultCubemap->vkSampler();
		irradianceMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo prefilterMapInfo{};
		prefilterMapInfo.imageView = present ? mvk::getImageView(skybox->prefilterMap()) : defaultCubemap->vkImageView();
		prefilterMapInfo.sampler = present ? mvk::getSampler(skybox->prefilterMap()) : defaultCubemap->vkSampler();
		prefilterMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo brdfInfo{};
		brdfInfo.imageView = present ? mvk::getImageView(skybox->brdfMap()) : defaultTexture->vkImageView();
		brdfInfo.sampler = present ? mvk::getSampler(skybox->brdfMap()) : defaultTexture->vkSampler();
		brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeDescriptors[3];

		writeDescriptors[0] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(3, descriptorSet, 1, &irradianceMapInfo);
		writeDescriptors[1] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(4, descriptorSet, 1, &prefilterMapInfo);
		writeDescriptors[2] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(5, descriptorSet, 1, &brdfInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 3, writeDescriptors, 0, nullptr));
	}

	void VulkanPBRForwardRenderPass::bindSceneDescriptorSet(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
		VkDescriptorSet descriptorSet = m_SceneDescriptorPool->get(imageIndex);
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout(),
										 0, 1, &descriptorSet, 0, nullptr));
	}

	void VulkanPBRForwardRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Clear});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanPBRForwardRenderPass::createCameraUniformBuffer() {
		m_CameraUniformBuffer = new VulkanUniformBuffer<CameraData>();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createRendererDataUniformBuffer() {
		m_RenderUniformBuffer = new VulkanUniformBuffer<RendererData>();
		m_RenderUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createSceneDataUniformBuffer() {
		m_SceneUniformBuffer = new VulkanUniformBuffer<SceneData>();
		m_SceneUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		createInfo.numSets = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

		m_SceneDescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_SceneDescriptorPool = mvk::DescriptorSet::Pool::create(m_SceneDescriptorSetLayout, createInfo);
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorSets() {

		m_SceneDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraBufferInfo{};
			cameraBufferInfo.offset = 0;
			cameraBufferInfo.range = sizeof(CameraData);
			cameraBufferInfo.buffer = m_CameraUniformBuffer->vkBuffer();

			VkDescriptorBufferInfo renderBufferInfo{};
			renderBufferInfo.offset = 0;
			renderBufferInfo.range = sizeof(RendererData);
			renderBufferInfo.buffer = m_RenderUniformBuffer->vkBuffer();

			VkDescriptorBufferInfo sceneBufferInfo{};
			sceneBufferInfo.offset = 0;
			sceneBufferInfo.range = sizeof(SceneData);
			sceneBufferInfo.buffer = m_SceneUniformBuffer->vkBuffer();

			const VulkanBuffer* visibleLightsBuffer = (const VulkanBuffer*)WorldRenderer::get().resources()
					.getBuffer(LightCullingPass::getVisibleLightsBufferHandle(index)).get();

			VkDescriptorBufferInfo visibleLightsBufferInfo{};
			visibleLightsBufferInfo.offset = 0;
			visibleLightsBufferInfo.range = sizeof(VisibleLightsIndices);
			visibleLightsBufferInfo.buffer = visibleLightsBuffer->vkBuffer();

			VkWriteDescriptorSet writeDescriptors[] = {
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &cameraBufferInfo),
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(1, descriptorSet, 1, &renderBufferInfo),
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(2, descriptorSet, 1, &sceneBufferInfo),
					mvk::WriteDescriptorSet::createDynamicStorageBufferWrite(3, descriptorSet, 1, &visibleLightsBufferInfo)
			};

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 4, writeDescriptors, 0, nullptr));
		});
	}

	void VulkanPBRForwardRenderPass::createSkyboxDescriptorLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		createInfo.numSets = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_SkyboxDescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_SkyboxDescriptorPool = mvk::DescriptorSet::Pool::create(m_SkyboxDescriptorSetLayout, createInfo);
	}

	void VulkanPBRForwardRenderPass::createSkyboxDescriptorSets() {
		m_SkyboxDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createShadowsDescriptorLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		createInfo.numSets = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_ShadowsDescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_ShadowsDescriptorPool = mvk::DescriptorSet::Pool::create(m_ShadowsDescriptorSetLayout, createInfo);
	}

	void VulkanPBRForwardRenderPass::createShadowsDescriptorSets() {
		m_ShadowsDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		VkPushConstantRange pushConstants{};
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);

		auto& materialResourcePool = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		pipelineInfo.pushConstantRanges.push_back(pushConstants);

		pipelineInfo.setLayouts.push_back(m_SceneDescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(materialResourcePool.materialDescriptorSetLayout());
		pipelineInfo.setLayouts.push_back(m_SkyboxDescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(m_ShadowsDescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaders.push_back({"resources/shaders/pbr/pbr.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/pbr/pbr.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanPBRForwardRenderPass", m_Device, pipelineInfo);
	}

	void VulkanPBRForwardRenderPass::createSemaphores() {
		mvk::Semaphore::create(m_SignalSemaphores.size(), m_SignalSemaphores.data());
	}
}
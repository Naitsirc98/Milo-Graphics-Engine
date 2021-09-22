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

		createSceneUniformBuffers();
		createSceneDescriptorSetLayoutAndPool();

		createShadowsUniformBuffer();
		createShadowsDescriptorSetLayoutAndPool();

		createGraphicsPipeline();
		createSemaphores();

		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	VulkanPBRForwardRenderPass::~VulkanPBRForwardRenderPass() {

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyRenderPass(device, m_RenderPass, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_EnvironmentUniformBuffer);
		DELETE_PTR(m_PointLightsUniformBuffer);
		DELETE_PTR(m_ShadowsUniformBuffer);
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

		MILO_PROFILE_FUNCTION;

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

		VkClearValue clearValues[2];
		clearValues[0].color = {0, 0, 0, 0};
		clearValues[1].depthStencil = {1.0f, 0};

		beginInfo.clearValues = clearValues;
		beginInfo.clearValuesCount = 2;

		mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);
		{
			renderScene(imageIndex, commandBuffer, scene);
		}
		mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
	}

	inline void VulkanPBRForwardRenderPass::renderScene(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		auto& materialResources = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		updateSceneUniformData(imageIndex, scene);
		updateShadowsUniformData(imageIndex, scene);
		bindDescriptorSets(imageIndex, commandBuffer);

		Mesh* lastMesh = nullptr;
		Material* lastMaterial = nullptr;

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
		VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(),
									VK_SHADER_STAGE_VERTEX_BIT,
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
										 2, 1, descriptorSets, 1, &dynamicOffset));
	}

	inline void VulkanPBRForwardRenderPass::updateSceneUniformData(uint32_t imageIndex, Scene* scene) {

		// ==== CAMERA
		const auto& camera = WorldRenderer::get().camera();
		{
			CameraData cameraData;
			cameraData.viewProjection = camera.projView;
			cameraData.view = camera.view;
			cameraData.position = Vector4(camera.position, 1.0f);

			m_CameraUniformBuffer->update(imageIndex, cameraData);
		}

		// ======== ENVIRONMENT

		const auto& lights = WorldRenderer::get().lights();
		Skybox* skybox = lights.skybox;

		{
			EnvironmentData env{};

			if(lights.dirLight.has_value()) {
				env.dirLight = lights.dirLight.value();
				env.dirLightPresent[0] = true;
			}

			if(skybox != nullptr) {
				env.maxPrefilterLod = skybox->maxPrefilterLOD();
				env.prefilterLodBias = skybox->prefilterLODBias();
				env.skyboxPresent[0] = true;
			}

			m_EnvironmentUniformBuffer->update(imageIndex, env);
		}

		{
			PointLightsData pointLightsData{};
			uint32_t pointLightsCount = std::min(lights.pointLights.size(), (size_t)MAX_POINT_LIGHTS);
			memcpy(pointLightsData.pointLights, lights.pointLights.data(), sizeof(PointLight) * pointLightsCount);
			pointLightsData.u_PointLightsCount = pointLightsCount;

			m_PointLightsUniformBuffer->update(imageIndex, pointLightsData);
		}

		// ==== SKYBOX TEXTURES

		if(skybox == nullptr) {
			setNullSkyboxUniformData(imageIndex);
			m_LastSkyboxModificationCount[imageIndex] = 0;
		} else if(skybox->modifications() != m_LastSkyboxModificationCount[imageIndex]) {
			setSkyboxUniformData(imageIndex, skybox);
			m_LastSkyboxModificationCount[imageIndex] = skybox->modifications();
		}
	}

	void VulkanPBRForwardRenderPass::setSkyboxUniformData(uint32_t imageIndex, Skybox* skybox) {

		VulkanCubemap* environmentMap = (VulkanCubemap*)skybox->environmentMap();
		VulkanCubemap* irradianceMap = (VulkanCubemap*)skybox->irradianceMap();
		VulkanCubemap* prefilterMap = (VulkanCubemap*)skybox->prefilterMap();
		VulkanTexture2D* brdfMap = (VulkanTexture2D*)skybox->brdfMap();

		VkDescriptorImageInfo irradianceMapInfo{};
		irradianceMapInfo.imageView = irradianceMap->vkImageView();
		irradianceMapInfo.sampler = irradianceMap->vkSampler();
		irradianceMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo prefilterMapInfo{};
		prefilterMapInfo.imageView = prefilterMap->vkImageView();
		prefilterMapInfo.sampler = prefilterMap->vkSampler();
		prefilterMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo brdfInfo{};
		brdfInfo.imageView = brdfMap->vkImageView();
		brdfInfo.sampler = brdfMap->vkSampler();
		brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorSet descriptorSet = m_SceneDescriptorPool->get(imageIndex);

		VkWriteDescriptorSet writeDescriptors[3];

		writeDescriptors[0] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(4, descriptorSet, 1, &irradianceMapInfo);
		writeDescriptors[1] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(5, descriptorSet, 1, &prefilterMapInfo);
		writeDescriptors[2] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(6, descriptorSet, 1, &brdfInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 3, writeDescriptors, 0, nullptr));
	}

	void VulkanPBRForwardRenderPass::setNullSkyboxUniformData(uint32_t imageIndex) {

		auto* defaultCubemap = dynamic_cast<VulkanCubemap*>(Assets::textures().blackCubemap().get());
		auto* defaultTexture = dynamic_cast<VulkanTexture2D*>(Assets::textures().blackTexture().get());

		VkDescriptorImageInfo irradianceMapInfo{};
		irradianceMapInfo.imageView = defaultCubemap->vkImageView();
		irradianceMapInfo.sampler = defaultCubemap->vkSampler();
		irradianceMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorImageInfo prefilterMapInfo = irradianceMapInfo;

		VkDescriptorImageInfo brdfInfo{};
		brdfInfo.imageView = defaultTexture->vkImageView();
		brdfInfo.sampler = defaultTexture->vkSampler();
		brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkDescriptorSet descriptorSet = m_SceneDescriptorPool->get(imageIndex);

		VkWriteDescriptorSet writeDescriptors[3];

		writeDescriptors[0] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(4, descriptorSet, 1, &irradianceMapInfo);
		writeDescriptors[1] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(5, descriptorSet, 1, &prefilterMapInfo);
		writeDescriptors[2] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(6, descriptorSet, 1, &brdfInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 3, writeDescriptors, 0, nullptr));
	}

	void VulkanPBRForwardRenderPass::updateShadowsUniformData(uint32_t imageIndex, Scene* scene) {

		{
			const auto& cascades = WorldRenderer::get().shadowCascades();

			ShadowDetails shadows{};
			shadows.u_SoftShadows[0] = WorldRenderer::get().softShadows();
			shadows.u_ShadowFade = 1;
			shadows.u_MaxShadowDistance = WorldRenderer::get().shadowsMaxDistance();
			shadows.u_LightSize = 0.5f;
			shadows.u_CascadeFading[0] = WorldRenderer::get().shadowCascadeFading();
			shadows.u_CascadeTransitionFade = WorldRenderer::get().shadowCascadeFadingValue();
			shadows.u_TilesCountX = (uint32_t) (scene->viewportSize().width / TILE_SIZE);
			shadows.u_ShowLightComplexity[0] = false;
			shadows.u_ShadowsEnabled[0] = WorldRenderer::get().shadowsEnabled();
			shadows.u_ShowCascades[0] = WorldRenderer::get().showShadowCascades();

			for(int32_t i = 0;i < cascades.size();++i) {
				shadows.u_LightMatrix[i] = cascades[i].viewProj;
				shadows.u_CascadeSplits[i] = cascades[i].splitDepth;
			}

			m_ShadowsUniformBuffer->update(imageIndex, shadows);
		}

		auto& resources = WorldRenderer::get().resources();

		VulkanTexture2DArray* shadowMap = (VulkanTexture2DArray*)resources.getTexture2D(ShadowMapRenderPass::getDepthMap(0)).get();

		shadowMap->setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

		VkDescriptorImageInfo imageInfos[4];
		VkWriteDescriptorSet writeDescriptors[4];

		VkDescriptorSet descriptorSet = m_ShadowsDescriptorPool->get(imageIndex);

		for(uint32_t i = 0;i < 4;++i) {

			imageInfos[i].imageView = shadowMap->vkImageView();
			imageInfos[i].sampler = shadowMap->vkSampler();
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

			writeDescriptors[i] = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(1, descriptorSet, 1, &imageInfos[i]);
			//writeDescriptors[i].dstArrayElement = i;
		}

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 4, writeDescriptors, 0, nullptr));
	}

	void VulkanPBRForwardRenderPass::bindDescriptorSets(uint32_t imageIndex, VkCommandBuffer commandBuffer) {

		VkDescriptorSet sceneDescriptorSet = m_SceneDescriptorPool->get(imageIndex);
		VkDescriptorSet shadowsDescriptorSet = m_ShadowsDescriptorPool->get(imageIndex);

		VkDescriptorSet descriptorSets[] = {
				sceneDescriptorSet,
				shadowsDescriptorSet
		};

		const VulkanStorageBuffer<VisibleLightsBuffer>* visibleLightsBuffer =
				(const VulkanStorageBuffer<VisibleLightsBuffer>*)WorldRenderer::get().resources()
				.getBuffer(LightCullingPass::getVisibleLightsBufferHandle()).get();

		uint32_t dynamicOffsets[] = {
				imageIndex * m_CameraUniformBuffer->elementSize(),
				imageIndex * m_EnvironmentUniformBuffer->elementSize(),
				imageIndex * m_PointLightsUniformBuffer->elementSize(),
				imageIndex * visibleLightsBuffer->elementSize(),
				imageIndex * m_ShadowsUniformBuffer->elementSize()
		};

		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										 m_GraphicsPipeline->pipelineLayout(),
										 0, 2, descriptorSets, 5, dynamicOffsets));
	}

	void VulkanPBRForwardRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Clear});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanPBRForwardRenderPass::createSceneUniformBuffers() {

		m_CameraUniformBuffer = VulkanUniformBuffer<CameraData>::create();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);

		m_EnvironmentUniformBuffer = VulkanUniformBuffer<EnvironmentData>::create();
		m_EnvironmentUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);

		m_PointLightsUniformBuffer = VulkanUniformBuffer<PointLightsData>::create();
		m_PointLightsUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorSetLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.numSets = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Camera
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		// Environment
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		// Point Lights
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
		// Visible Lights buffer
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
		// Skybox textures
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_SceneDescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_SceneDescriptorPool = mvk::DescriptorSet::Pool::create(m_SceneDescriptorSetLayout, createInfo);

		m_SceneDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraInfo{};
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraData);
			cameraInfo.buffer = m_CameraUniformBuffer->vkBuffer();

			VkDescriptorBufferInfo environmentInfo{};
			environmentInfo.offset = 0;
			environmentInfo.range = sizeof(EnvironmentData);
			environmentInfo.buffer = m_EnvironmentUniformBuffer->vkBuffer();

			VkDescriptorBufferInfo pointLightsInfo{};
			pointLightsInfo.offset = 0;
			pointLightsInfo.range = sizeof(PointLightsData);
			pointLightsInfo.buffer = m_PointLightsUniformBuffer->vkBuffer();

			const VulkanStorageBuffer<VisibleLightsBuffer>* visibleLightsBuffer =
					(const VulkanStorageBuffer<VisibleLightsBuffer>*)WorldRenderer::get().resources()
							.getBuffer(LightCullingPass::getVisibleLightsBufferHandle()).get();

			VkDescriptorBufferInfo visibleLightsInfo{};
			visibleLightsInfo.offset = 0;
			visibleLightsInfo.range = sizeof(VisibleLightsBuffer);
			visibleLightsInfo.buffer = visibleLightsBuffer->vkBuffer();

			VkWriteDescriptorSet writeDescriptors[] = {
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &cameraInfo),
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(1, descriptorSet, 1, &environmentInfo),
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(2, descriptorSet, 1, &pointLightsInfo),
					mvk::WriteDescriptorSet::createDynamicStorageBufferWrite(3, descriptorSet, 1, &visibleLightsInfo)
			};

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 4, writeDescriptors, 0, nullptr));
		});
	}

	void VulkanPBRForwardRenderPass::createShadowsUniformBuffer() {
		m_ShadowsUniformBuffer = VulkanUniformBuffer<ShadowDetails>::create();
		m_ShadowsUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createShadowsDescriptorSetLayoutAndPool() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};
		// Shadow details
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Shadow maps
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pBindings = bindings.data();
		layoutInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &layoutInfo, nullptr, &m_ShadowsDescriptorSetLayout));

		VulkanDescriptorPool::CreateInfo poolInfo{};
		poolInfo.layout = m_ShadowsDescriptorSetLayout;
		poolInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		poolInfo.poolSizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, MAX_SWAPCHAIN_IMAGE_COUNT});
		poolInfo.poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 * MAX_SWAPCHAIN_IMAGE_COUNT});

		m_ShadowsDescriptorPool = new VulkanDescriptorPool(m_Device, poolInfo);

		m_ShadowsDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo shadowsInfo{};
			shadowsInfo.offset = 0;
			shadowsInfo.range = sizeof(ShadowDetails);
			shadowsInfo.buffer = m_ShadowsUniformBuffer->vkBuffer();

			VkWriteDescriptorSet writeDescriptors[] = {
					mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &shadowsInfo)
			};

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, writeDescriptors, 0, nullptr));
		});
	}

	void VulkanPBRForwardRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setVertexAttributes(VERTEX_ALL_ATTRIBUTES);

		VkPushConstantRange pushConstants{};
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);

		auto& materialResourcePool = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		pipelineInfo.pushConstantRanges.push_back(pushConstants);

		pipelineInfo.setLayouts.push_back(m_SceneDescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(m_ShadowsDescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(materialResourcePool.materialDescriptorSetLayout());

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
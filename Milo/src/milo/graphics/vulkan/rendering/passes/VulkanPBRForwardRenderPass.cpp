#include <milo/graphics/vulkan/buffers/VulkanMeshBuffers.h>
#include "milo/graphics/vulkan/rendering/passes/VulkanPBRForwardRenderPass.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/vulkan/textures/VulkanCubemap.h"
#include "milo/graphics/rendering/WorldRenderer.h"

namespace milo {

	VulkanPBRForwardRenderPass::VulkanPBRForwardRenderPass() {

		m_Device = VulkanContext::get()->device();

		createRenderPass();

		createCameraUniformBuffer();
		createLightsUniformBuffer();
		createSkyboxUniformBuffer();
		createSceneDescriptorLayout();
		createSceneDescriptorPool();
		createSceneDescriptorSets();

		createGraphicsPipeline();

		createCommandBuffers();
		createSemaphores();
	}

	VulkanPBRForwardRenderPass::~VulkanPBRForwardRenderPass() {

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyRenderPass(device, m_RenderPass, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_LightsUniformBuffer);
		DELETE_PTR(m_SkyboxUniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_SceneDescriptorSetLayout, nullptr));
		DELETE_PTR(m_SceneDescriptorPool);

		DELETE_PTR(m_GraphicsPipeline);

		DELETE_PTR(m_CommandPool);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALLV(vkDestroySemaphore(device, m_SignalSemaphores[i], nullptr));
		}
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

		Camera* camera = scene->camera();
		Transform& cameraTransform = scene->cameraEntity().getComponent<Transform>();

		beginCommandBuffer(commandBuffer);
		{
			beginRenderPass(imageIndex, commandBuffer);
			{
				bindGraphicsPipeline(commandBuffer);

				updateCameraUniformData(imageIndex, camera, cameraTransform);
				updateLightsUniformData(imageIndex, scene->lightEnvironment());
				updateSkyboxUniformData(imageIndex, scene->skybox());

				bindSceneDescriptorSet(commandBuffer, imageIndex);

				renderScene(commandBuffer, imageIndex, scene);
			}
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		}
		VK_CALLV(vkEndCommandBuffer(commandBuffer));
	}

	inline void VulkanPBRForwardRenderPass::renderScene(VkCommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene) const {

		auto& materialResources = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		Mesh* lastMesh = nullptr;
		Material* lastMaterial = nullptr;

		VkDescriptorSet sceneDescriptorSet = m_SceneDescriptorPool->get(imageIndex);

		auto components = scene->group<Transform, MeshView>();
		for(EntityId entityId : components) {

			const Transform& transform = components.get<Transform>(entityId);
			const MeshView& meshView = components.get<MeshView>(entityId);

			if(lastMaterial != meshView.material) {

				uint32_t dynamicOffset;

				VkDescriptorSet materialDescriptorSet = materialResources.descriptorSetOf(meshView.material, dynamicOffset);
				VkDescriptorSet descriptorSets[] = {materialDescriptorSet, sceneDescriptorSet};

				VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
												 m_GraphicsPipeline->pipelineLayout(),
												 0, 2, descriptorSets, 1, &dynamicOffset));

				lastMaterial = meshView.material;
			}

			if(lastMesh != meshView.mesh) {

				VulkanMeshBuffers* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(meshView.mesh->buffers());

				VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
				VkDeviceSize offsets[] = {0};
				VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

				if(!meshView.mesh->indices().empty()) {
					VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
				}

				lastMesh = meshView.mesh;
			}

			PushConstants pushConstants = {transform.modelMatrix()};
			VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
										0, sizeof(PushConstants), &pushConstants));

			if(meshView.mesh->indices().empty()) {
				VK_CALLV(vkCmdDraw(commandBuffer, meshView.mesh->vertices().size(), 1, 0, 0));
			} else {
				VK_CALLV(vkCmdDrawIndexed(commandBuffer, meshView.mesh->indices().size(), 1, 0, 0, 0));
			}
		}
	}

	inline void VulkanPBRForwardRenderPass::updateCameraUniformData(uint32_t imageIndex, const Camera* camera, const Transform& cameraTransform) {

		CameraUniformBuffer cameraData{};
		cameraData.viewMatrix = camera->viewMatrix(cameraTransform.translation);
		cameraData.projMatrix = camera->projectionMatrix();
		cameraData.position = Vector4(cameraTransform.translation, 1.0f);
		cameraData.projViewMatrix = cameraData.projMatrix * cameraData.viewMatrix;

		m_CameraUniformBuffer->update(imageIndex, cameraData);
	}

	void VulkanPBRForwardRenderPass::updateLightsUniformData(uint32_t imageIndex, const LightEnvironment& lights) {

		LightsUniformBuffer lightsData{};

		lightsData.directionalLight = lights.directionalLight;
		memcpy(lightsData.pointLights, lights.pointLights.data(), lights.pointLights.size() * sizeof(PointLight));
		lightsData.u_AmbientColor = lights.ambientColor;
		lightsData.pointLightsCount = lights.pointLights.size();

		m_LightsUniformBuffer->update(imageIndex, lightsData);
	}

	void VulkanPBRForwardRenderPass::updateSkyboxUniformData(uint32_t imageIndex, const Skybox* skybox) {

		SkyboxUniformBuffer skyboxData{};

		if(skybox != nullptr) {
			skyboxData.prefilterLODBias = skybox->prefilterLODBias();
			skyboxData.maxPrefilterLOD = skybox->maxPrefilterLOD();
			skyboxData.present = true;
		}

		m_SkyboxUniformBuffer->update(imageIndex, skyboxData);

		bool present = skyboxData.present;
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

	inline void VulkanPBRForwardRenderPass::bindGraphicsPipeline(VkCommandBuffer commandBuffer) const {

		VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

		Size size = Window::get()->size();

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)size.width;
		viewport.height = (float)size.height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = {(uint32_t)size.width, (uint32_t)size.height};

		VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
		VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));
	}

	inline void VulkanPBRForwardRenderPass::beginRenderPass(uint32_t imageIndex, VkCommandBuffer commandBuffer) {

		auto& framebuffer = dynamic_cast<VulkanFramebuffer&>(WorldRenderer::get().getFramebuffer());

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.framebuffer = framebuffer.get(m_RenderPass);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent.width = framebuffer.size().width;
		renderPassInfo.renderArea.extent.height = framebuffer.size().height;

		VkClearValue clearValues[2];
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1};
		clearValues[1].depthStencil = {1.0f, 0};

		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;

		VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
	}

	inline void VulkanPBRForwardRenderPass::beginCommandBuffer(VkCommandBuffer commandBuffer) const {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	}

	void VulkanPBRForwardRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);

		VkAttachmentDescription depthAttachment = mvk::AttachmentDescription::createDepthStencilAttachment();

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.colorAttachmentCount = 1;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.subpassCount = 1;

		VK_CALL(vkCreateRenderPass(m_Device->logical(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void VulkanPBRForwardRenderPass::createCameraUniformBuffer() {

		m_CameraUniformBuffer = new VulkanUniformBuffer<CameraUniformBuffer>();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createLightsUniformBuffer() {
		m_LightsUniformBuffer = new VulkanUniformBuffer<LightsUniformBuffer>();
		m_LightsUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createSkyboxUniformBuffer() {
		m_SkyboxUniformBuffer = new VulkanUniformBuffer<SkyboxUniformBuffer>();
		m_SkyboxUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorLayout() {

		Array<VkDescriptorSetLayoutBinding, 6> bindings{};
		// Camera
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Lights
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[1].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Skybox
		bindings[2].binding = 2;
		bindings[2].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[2].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// IrradianceMap
		bindings[3].binding = 3;
		bindings[3].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[3].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// PrefilterMap
		bindings[4].binding = 4;
		bindings[4].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[4].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// BRDF
		bindings[5].binding = 5;
		bindings[5].stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[5].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_SceneDescriptorSetLayout));
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorPool() {

		Array<VkDescriptorPoolSize, 6> poolSizes{};
		// Camera
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Lights
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Skybox
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Irradiance map
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[3].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Prefilter map
		poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[4].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// BRDF
		poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[5].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_SceneDescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.poolSizes.insert(createInfo.poolSizes.end(), poolSizes.begin(), poolSizes.end());

		m_SceneDescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanPBRForwardRenderPass::createSceneDescriptorSets() {

		m_SceneDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraBufferInfo{};
			cameraBufferInfo.buffer = m_CameraUniformBuffer->vkBuffer();
			cameraBufferInfo.offset = index * m_CameraUniformBuffer->elementSize();
			cameraBufferInfo.range = sizeof(CameraUniformBuffer);

			VkDescriptorBufferInfo lightsBufferInfo{};
			lightsBufferInfo.buffer = m_LightsUniformBuffer->vkBuffer();
			lightsBufferInfo.offset = index * m_LightsUniformBuffer->elementSize();
			lightsBufferInfo.range = sizeof(LightsUniformBuffer);

			VkDescriptorBufferInfo skyboxBufferInfo{};
			skyboxBufferInfo.buffer = m_SkyboxUniformBuffer->vkBuffer();
			skyboxBufferInfo.offset = index * m_SkyboxUniformBuffer->elementSize();
			skyboxBufferInfo.range = sizeof(SkyboxUniformBuffer);

			Array<VkWriteDescriptorSet, 3> writeDescriptors{};
			// Camera
			writeDescriptors[0] = mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &cameraBufferInfo);
			// Lights
			writeDescriptors[1] = mvk::WriteDescriptorSet::createUniformBufferWrite(1, descriptorSet, 1, &lightsBufferInfo);
			// Skybox
			writeDescriptors[2] = mvk::WriteDescriptorSet::createUniformBufferWrite(2, descriptorSet, 1, &skyboxBufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), writeDescriptors.size(), writeDescriptors.data(), 0, nullptr));
		});
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

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/pbr/pbr.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/pbr/pbr.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanPBRForwardRenderPass", m_Device, pipelineInfo);
	}

	void VulkanPBRForwardRenderPass::createCommandBuffers() {

		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_CommandBuffers);
	}

	void VulkanPBRForwardRenderPass::createSemaphores() {
		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}
}
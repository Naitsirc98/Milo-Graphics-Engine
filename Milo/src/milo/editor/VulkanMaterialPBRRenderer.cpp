#include "milo/editor/VulkanMaterialViewerRenderer.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	VulkanMaterialPBRRenderer::VulkanMaterialPBRRenderer(VulkanFramebuffer* framebuffer) : m_Framebuffer(framebuffer) {
		m_Device = m_Framebuffer->device();
		createRenderPass();
		createUniformBuffers();
		createDescriptorSetLayoutAndPool();
		createGraphicsPipeline();
	}

	VulkanMaterialPBRRenderer::~VulkanMaterialPBRRenderer() {

		if(m_CommandBuffer != VK_NULL_HANDLE)
			m_Device->graphicsCommandPool()->free(1, &m_CommandBuffer);

		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_EnvironmentUniformBuffer);
		DELETE_PTR(m_GraphicsPipeline);

		VK_CALLV(vkDestroyRenderPass(m_Device->logical(), m_RenderPass, nullptr));
	}

	void VulkanMaterialPBRRenderer::render(Material* material) {

		if(m_Material == nullptr || m_Material->name() != material->name() || material->dirty()) {
			m_Material = material;
			buildCommandBuffer();
		}

		renderMaterial();
	}

	void VulkanMaterialPBRRenderer::renderMaterial() {

		VulkanQueue* queue = m_Device->graphicsQueue();

		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitDstStageMask = &waitStageMask;

		queue->submit(submitInfo, nullptr);
		queue->awaitTermination();
	}

	static Vector4 getCameraPosition() {
		return {0, 0, -5, 1};
	}

	static Matrix4 getViewMatrix() {
		return glm::lookAt(Vector3(getCameraPosition()), Vector3(0, 0, 0), Vector3(0, 1, 0));
	}

	static Matrix4 getViewProjectionMatrix(const Size& size) {
		return glm::perspective(radians(45.0f), size.aspect(), 0.1f, 100.0f) * getViewMatrix();
	}

	static DirectionalLight getDirLight() {
		DirectionalLight light{};
		light.castShadows = false;
		light.direction = normalize(Vector3(0.1f, 0.8f, 0.1f));
		return light;
	}

	void VulkanMaterialPBRRenderer::createUniformBuffers() {

		m_CameraUniformBuffer = VulkanUniformBuffer<CameraData>::create();
		m_CameraUniformBuffer->allocate(1);

		CameraData camera{};
		camera.view = getViewMatrix();
		camera.viewProjection = getViewProjectionMatrix(m_Framebuffer->size());
		camera.position = getCameraPosition();

		m_CameraUniformBuffer->update(0, camera);

		m_EnvironmentUniformBuffer = VulkanUniformBuffer<EnvironmentData>::create();
		m_EnvironmentUniformBuffer->allocate(1);

		EnvironmentData env{};
		env.dirLight = getDirLight();
		env.prefilterLodBias = -0.5f;
		env.maxPrefilterLod = 1.0f;
		env.dirLightPresent[0] = true;
		env.skyboxPresent[0] = true;

		m_EnvironmentUniformBuffer->update(0, env);
	}

	void VulkanMaterialPBRRenderer::createDescriptorSetLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.numSets = 1;
		createInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Camera
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		// Environment
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		// Skybox textures
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		m_DescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_DescriptorPool = mvk::DescriptorSet::Pool::create(m_DescriptorSetLayout, createInfo);

		m_DescriptorPool->allocate(1, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo cameraInfo{};
			cameraInfo.offset = 0;
			cameraInfo.range = sizeof(CameraData);
			cameraInfo.buffer = m_CameraUniformBuffer->vkBuffer();

			VkDescriptorBufferInfo environmentInfo{};
			environmentInfo.offset = 0;
			environmentInfo.range = sizeof(EnvironmentData);
			environmentInfo.buffer = m_EnvironmentUniformBuffer->vkBuffer();

			Skybox* skybox = Assets::skybox().getIndoorSkybox();
			VulkanCubemap* irradiance = (VulkanCubemap*) skybox->irradianceMap();
			VulkanCubemap* prefilter = (VulkanCubemap*) skybox->prefilterMap();
			VulkanTexture2D* brdf = (VulkanTexture2D*) skybox->brdfMap();

			VkDescriptorImageInfo irradianceInfo{};
			irradianceInfo.imageView = irradiance->vkImageView();
			irradianceInfo.sampler = irradiance->vkSampler();
			irradianceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorImageInfo prefilterInfo{};
			prefilterInfo.imageView = prefilter->vkImageView();
			prefilterInfo.sampler = prefilter->vkSampler();
			prefilterInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkDescriptorImageInfo brdfInfo{};
			brdfInfo.imageView = brdf->vkImageView();
			brdfInfo.sampler = brdf->vkSampler();
			brdfInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet writeDescriptors[] = {
					mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &cameraInfo),
					mvk::WriteDescriptorSet::createUniformBufferWrite(1, descriptorSet, 1, &environmentInfo),
					mvk::WriteDescriptorSet::createCombineImageSamplerWrite(2, descriptorSet, 1, &irradianceInfo),
					mvk::WriteDescriptorSet::createCombineImageSamplerWrite(3, descriptorSet, 1, &prefilterInfo),
					mvk::WriteDescriptorSet::createCombineImageSamplerWrite(4, descriptorSet, 1, &brdfInfo)
			};

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 5, writeDescriptors, 0, nullptr));
		});
	}

	void VulkanMaterialPBRRenderer::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, RenderPass::LoadOp::Clear});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, RenderPass::LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanMaterialPBRRenderer::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setVertexAttributes(VERTEX_ALL_ATTRIBUTES);

		auto& materialResourcePool = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(materialResourcePool.materialDescriptorSetLayout());

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaders.push_back({"resources/shaders/material/material-pbr.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/material/material-pbr.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.viewport.x = 0;
		pipelineInfo.viewport.y = 0;
		pipelineInfo.viewport.width = m_Framebuffer->size().width;
		pipelineInfo.viewport.height = m_Framebuffer->size().height;
		pipelineInfo.viewport.maxDepth = 1.0f;
		pipelineInfo.viewport.minDepth = 0.0f;

		pipelineInfo.scissor.offset = {0, 0};
		pipelineInfo.scissor.extent = {(uint32_t)m_Framebuffer->size().width, (uint32_t)m_Framebuffer->size().height};

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanMaterialPBRRenderer", m_Device, pipelineInfo);
	}

	void VulkanMaterialPBRRenderer::bindDescriptorSets() {

		VulkanMaterialResourcePool& materialResourcePool = (VulkanMaterialResourcePool&) Assets::materials().resourcePool();

		uint32_t dynamicOffset;
		VkDescriptorSet descriptorSets[] = {m_DescriptorPool->get(0), materialResourcePool.descriptorSetOf(m_Material, dynamicOffset)};

		VK_CALLV(vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout(),
										0, 2, descriptorSets, 1, &dynamicOffset));
	}

	void VulkanMaterialPBRRenderer::buildCommandBuffer() {

		if(m_CommandBuffer != VK_NULL_HANDLE) {
			m_Device->graphicsCommandPool()->free(1, &m_CommandBuffer);
		}
		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &m_CommandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass;
			renderPassInfo.framebuffer = m_Framebuffer->get(m_RenderPass);
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = {(uint32_t)m_Framebuffer->size().width, (uint32_t)m_Framebuffer->size().height};

			VkClearValue clearValues[2]{};
			clearValues[0].color = {1, 0, 0, 0};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues;

			VK_CALLV(vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
			{
				VK_CALLV(vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

				bindDescriptorSets();

				Mesh* mesh = Assets::meshes().getSphere();
				VulkanMeshBuffers* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());

				VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
				VkDeviceSize offsets[] = {0};
				VK_CALLV(vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, vertexBuffers, offsets));

				if(!mesh->indices().empty()) {
					VK_CALLV(vkCmdBindIndexBuffer(m_CommandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
				}

				if(mesh->indices().empty()) {
					VK_CALLV(vkCmdDraw(m_CommandBuffer, mesh->vertices().size(), 1, 0, 0));
				} else {
					VK_CALLV(vkCmdDrawIndexed(m_CommandBuffer, mesh->indices().size(), 1, 0, 0, 0));
				}
			}
			VK_CALLV(vkCmdEndRenderPass(m_CommandBuffer));

		}
		VK_CALL(vkEndCommandBuffer(m_CommandBuffer));
	}
}
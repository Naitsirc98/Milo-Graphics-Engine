#include <milo/graphics/vulkan/buffers/VulkanMeshBuffers.h>
#include "milo/editor/VulkanMaterialViewerRenderer.h"

namespace milo {

	VulkanMaterialSkyboxRenderer::VulkanMaterialSkyboxRenderer(VulkanFramebuffer* framebuffer, MaterialViewerCameraData* camera)
		: m_Framebuffer(framebuffer), m_Camera(camera) {
		m_Device = m_Framebuffer->device();
		createRenderPass();
		createUniformBuffers();
		createDescriptorSetLayoutAndPool();
		createGraphicsPipeline();
		buildCommandBuffer();
	}

	VulkanMaterialSkyboxRenderer::~VulkanMaterialSkyboxRenderer() {

		if(m_CommandBuffer != VK_NULL_HANDLE)
			m_Device->graphicsCommandPool()->free(1, &m_CommandBuffer);

		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));

		DELETE_PTR(m_DescriptorPool);
		DELETE_PTR(m_UniformBuffer);
		DELETE_PTR(m_GraphicsPipeline);

		VK_CALLV(vkDestroyRenderPass(m_Device->logical(), m_RenderPass, nullptr));
	}

	void VulkanMaterialSkyboxRenderer::render() {

		VulkanQueue* queue = m_Device->graphicsQueue();

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &m_CommandBuffer;
		submitInfo.commandBufferCount = 1;

		queue->submit(submitInfo, nullptr);
		queue->awaitTermination();
	}

	void VulkanMaterialSkyboxRenderer::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, RenderPass::LoadOp::Load});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, RenderPass::LoadOp::Load};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanMaterialSkyboxRenderer::createUniformBuffers() {
		m_UniformBuffer = VulkanUniformBuffer<UniformBuffer>::create();
		m_UniformBuffer->allocate(1);
	}

	void VulkanMaterialSkyboxRenderer::createDescriptorSetLayoutAndPool() {

		Array<VkDescriptorSetLayoutBinding, 2> bindings{};
		// Uniform buffer
		bindings[0].binding = 0;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].descriptorCount = 1;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Cubemap
		bindings[1].binding = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bindings[1].descriptorCount = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = 2;

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));

		Array<VkDescriptorPoolSize, 2> poolSizes{};
		// Uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		// Cubemap
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		VulkanDescriptorPool::CreateInfo poolInfo{};
		poolInfo.layout = m_DescriptorSetLayout;
		poolInfo.capacity = 1;
		poolInfo.poolSizes.push_back(poolSizes[0]);
		poolInfo.poolSizes.push_back(poolSizes[1]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, poolInfo);

		m_DescriptorPool->allocate(1, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBuffer);

			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanMaterialSkyboxRenderer::createGraphicsPipeline() {
		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;
		pipelineInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		pipelineInfo.shaders.push_back({"resources/shaders/material/material-skybox.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/material/material-skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		// TODO: viewport & scissor

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanSkyboxRenderPass", m_Device, pipelineInfo);
	}

	void VulkanMaterialSkyboxRenderer::bindDescriptorSets() {

		Skybox* skybox = Assets::skybox().getIndoorSkybox();

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(0);
		updateDescriptorSets(skybox, descriptorSet);

		VK_CALLV(vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										 m_GraphicsPipeline->pipelineLayout(),
										 0, 1, &descriptorSet, 0, nullptr));
	}

	void VulkanMaterialSkyboxRenderer::updateDescriptorSets(Skybox* skybox, VkDescriptorSet descriptorSet) {
		auto* environmentMap = dynamic_cast<VulkanCubemap*>(skybox->environmentMap());

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer->vkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBuffer);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = environmentMap->vkImageView();
		imageInfo.sampler = environmentMap->vkSampler();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeDescriptors = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(1, descriptorSet, 1, &imageInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptors, 0, nullptr));
	}

	void VulkanMaterialSkyboxRenderer::buildCommandBuffer() {

		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1, &m_CommandBuffer);

		Skybox* skybox = Assets::skybox().getIndoorSkybox();

		UniformBuffer uniformBufferData{};
		uniformBufferData.viewMatrix = m_Camera->view;
		uniformBufferData.projMatrix = m_Camera->proj;
		uniformBufferData.textureLOD = skybox->prefilterLODBias();
		uniformBufferData.intensity = 1; // TODO

		m_UniformBuffer->update(0, uniformBufferData);

		Mesh* mesh = Assets::meshes().getCube();
		auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass;
			renderPassInfo.framebuffer = m_Framebuffer->get(m_RenderPass);
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent.width = m_Framebuffer->size().width;
			renderPassInfo.renderArea.extent.height = m_Framebuffer->size().height;

			VK_CALLV(vkCmdBeginRenderPass(m_CommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
			{
				VK_CALLV(vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

				bindDescriptorSets();

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
		VK_CALLV(vkEndCommandBuffer(m_CommandBuffer));
	}
}
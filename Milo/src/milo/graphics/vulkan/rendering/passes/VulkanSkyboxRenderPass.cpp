#include "milo/graphics/vulkan/rendering/passes/VulkanSkyboxRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/assets/AssetManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	VulkanSkyboxRenderPass::VulkanSkyboxRenderPass() {

		m_Device = VulkanContext::get()->device();

		createRenderPass();

		createDescriptorSetLayout();
		createDescriptorPool();
		createUniformBuffer();
		createDescriptorSets();

		createSemaphores();

		m_Framebuffers.fill(VK_NULL_HANDLE);
		m_CommandBuffers.fill(VK_NULL_HANDLE);
	}

	VulkanSkyboxRenderPass::~VulkanSkyboxRenderPass() {

		destroyTransientResources();

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyRenderPass(device, m_RenderPass, nullptr));

		DELETE_PTR(m_UniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr));
		DELETE_PTR(m_DescriptorPool);

		DELETE_PTR(m_GraphicsPipeline);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALLV(vkDestroySemaphore(device, m_SignalSemaphores[i], nullptr));
		}
	}

	void VulkanSkyboxRenderPass::compile(FrameGraphResourcePool* resourcePool) {

		destroyTransientResources();

		createFramebuffers(resourcePool);

		createGraphicsPipeline();

		createCommandBuffers();
	}

	void VulkanSkyboxRenderPass::execute(Scene* scene) {

		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* queue = m_Device->graphicsQueue();

		Camera* camera = scene->camera();

		UniformBuffer uniformBufferData{};
		uniformBufferData.viewMatrix = camera->viewMatrix(scene->cameraEntity().getComponent<Transform>().translation);
		uniformBufferData.projMatrix = camera->projectionMatrix();
		uniformBufferData.textureLOD = scene->skybox()->prefilterLODBias();
		uniformBufferData.intensity = 1; // TODO

		m_UniformBuffer->update(imageIndex, uniformBufferData);

		VkPipelineStageFlags waitDstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pWaitDstStageMask = &waitDstStageFlags;
		submitInfo.pSignalSemaphores = &m_SignalSemaphores[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;

		queue->submit(submitInfo, VK_NULL_HANDLE);
	}

	void VulkanSkyboxRenderPass::updateDescriptorSets(Skybox* skybox, uint32_t imageIndex, VkDescriptorSet descriptorSet) {

		auto* environmentMap = dynamic_cast<VulkanCubemap*>(skybox->environmentMap());

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer->vkBuffer();
		bufferInfo.offset = imageIndex * m_UniformBuffer->elementSize();
		bufferInfo.range = sizeof(UniformBuffer);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = environmentMap->vkImageView();
		imageInfo.sampler = environmentMap->vkSampler();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet writeDescriptors = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(1, descriptorSet, 1, &imageInfo);

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptors, 0, nullptr));
	}

	void VulkanSkyboxRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = mvk::AttachmentDescription::createDepthStencilAttachment();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.colorAttachmentCount = 1;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.subpassCount = 1;

		VK_CALL(vkCreateRenderPass(m_Device->logical(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void VulkanSkyboxRenderPass::createFramebuffers(FrameGraphResourcePool* resourcePool) {

		Size size = Window::get()->size();


		VulkanFrameGraphResourcePool* pool = dynamic_cast<VulkanFrameGraphResourcePool*>(resourcePool);

		for(uint32_t i = 0;i < m_Framebuffers.size();++i) {


			const VulkanTexture2D* colorTexture = dynamic_cast<const VulkanTexture2D*>(pool->getRenderTarget(i).colorAttachment);
			const VulkanTexture2D* depthTexture = dynamic_cast<const VulkanTexture2D*>(pool->getRenderTarget(i).depthAttachment);

			VkImageView attachments[2] = { colorTexture->vkImageView(), depthTexture->vkImageView() };

			VkFramebufferCreateInfo createInfo = mvk::FramebufferCreateInfo::create(m_RenderPass,
																					colorTexture->width(), colorTexture->height());
			createInfo.pAttachments = attachments;
			createInfo.attachmentCount = 2;

			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &createInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanSkyboxRenderPass::createDescriptorSetLayout() {

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
	}

	void VulkanSkyboxRenderPass::createDescriptorPool() {

		Array<VkDescriptorPoolSize, 2> poolSizes{};
		// Uniform buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;
		// Cubemap
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.poolSizes.push_back(poolSizes[0]);
		createInfo.poolSizes.push_back(poolSizes[1]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanSkyboxRenderPass::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<UniformBuffer>();
		m_UniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanSkyboxRenderPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = index * m_UniformBuffer->elementSize();
			bufferInfo.range = sizeof(UniformBuffer);

			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanSkyboxRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;
		pipelineInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/skybox/skybox.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/skybox/skybox.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanSkyboxRenderPass", m_Device, pipelineInfo);
	}

	void VulkanSkyboxRenderPass::createCommandBuffers() {

		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
												  m_CommandBuffers.size(), m_CommandBuffers.data());

		Scene* scene = SceneManager::activeScene();

		Skybox* skybox = scene->skybox();

		Entity cameraEntity = scene->cameraEntity();

		if(!cameraEntity.valid()) {
			// TODO
			throw MILO_RUNTIME_EXCEPTION("Main Camera Entity is not valid");
		}

		Camera& camera = cameraEntity.getComponent<Camera>();
		Matrix4 proj = camera.projectionMatrix();
		Matrix4 view = camera.viewMatrix(cameraEntity.getComponent<Transform>().translation);

		UniformBuffer uniformBufferData{};
		uniformBufferData.viewMatrix = view;
		uniformBufferData.projMatrix = proj;
		uniformBufferData.textureLOD = skybox->prefilterLODBias();
		uniformBufferData.intensity = 1; // TODO

		Mesh* mesh = Assets::meshes().getCube();

		auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());

		for(uint32_t imageIndex = 0; imageIndex < m_CommandBuffers.size(); ++imageIndex) {

			VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			const Viewport& sceneViewport = scene->viewport();

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
			{
				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_RenderPass;
				renderPassInfo.framebuffer = m_Framebuffers[imageIndex];
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent.width = fabs(sceneViewport.width);
				renderPassInfo.renderArea.extent.height = fabs(sceneViewport.height);

				VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
				{
					VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

					VkViewport viewport{};
					viewport.x = (float)sceneViewport.x;
					viewport.y = (float)sceneViewport.y;
					viewport.width = (float)sceneViewport.width;
					viewport.height = (float)sceneViewport.height;
					viewport.minDepth = 0;
					viewport.maxDepth = 1;

					VkRect2D scissor{};
					scissor.offset = {0, 0};
					scissor.extent = {(uint32_t)sceneViewport.width, (uint32_t)sceneViewport.height};

					VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
					VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

					m_UniformBuffer->update(imageIndex, uniformBufferData);

					VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);

					updateDescriptorSets(skybox, imageIndex, descriptorSet);

					VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
													 m_GraphicsPipeline->pipelineLayout(),
													 0, 1, &descriptorSet, 0, nullptr));

					VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
					VkDeviceSize offsets[] = {0};
					VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

					if(!mesh->indices().empty()) {
						VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
					}

					if(mesh->indices().empty()) {
						VK_CALLV(vkCmdDraw(commandBuffer, mesh->vertices().size(), 1, 0, 0));
					} else {
						VK_CALLV(vkCmdDrawIndexed(commandBuffer, mesh->indices().size(), 1, 0, 0, 0));
					}
				}
				VK_CALLV(vkCmdEndRenderPass(commandBuffer));
			}
			VK_CALLV(vkEndCommandBuffer(commandBuffer));
		}
	}

	void VulkanSkyboxRenderPass::createSemaphores() {

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}

	void VulkanSkyboxRenderPass::destroyTransientResources() {

		m_Device->awaitTermination();

		if(m_Framebuffers[0] != VK_NULL_HANDLE) {
			for(uint32_t i = 0; i < m_Framebuffers.size(); ++i) {
				VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), m_Framebuffers[i], nullptr));
				m_Framebuffers[i] = VK_NULL_HANDLE;
			}
		}

		if(m_CommandBuffers[0] != VK_NULL_HANDLE) {
			m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
			m_CommandBuffers.fill(VK_NULL_HANDLE);
		}
	}
}
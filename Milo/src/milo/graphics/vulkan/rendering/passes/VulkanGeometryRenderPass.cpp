#include "milo/graphics/vulkan/rendering/passes/VulkanGeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/scenes/Entity.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	VulkanGeometryRenderPass::VulkanGeometryRenderPass() {

		m_Device = VulkanContext::get()->device();

		createRenderPass();

		createCameraUniformBuffer();
		createCameraDescriptorLayout();
		createCameraDescriptorPool();
		createCameraDescriptorSets();

		createGraphicsPipeline();

		createCommandPool();
		createCommandBuffers();
		createSemaphores();
	}

	VulkanGeometryRenderPass::~VulkanGeometryRenderPass() {

		destroyTransientResources();

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyRenderPass(device, m_RenderPass, nullptr));

		DELETE_PTR(m_CameraUniformBuffer);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_CameraDescriptorSetLayout, nullptr));
		DELETE_PTR(m_CameraDescriptorPool);

		DELETE_PTR(m_GraphicsPipeline);

		DELETE_PTR(m_CommandPool);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALLV(vkDestroySemaphore(device, m_SignalSemaphores[i], nullptr));
		}
	}

	void VulkanGeometryRenderPass::destroyTransientResources() {

		m_Device->awaitTermination();

		for(uint32_t i = 0; i < m_Framebuffers.size(); ++i) {
			VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), m_Framebuffers[i], nullptr));
		}
	}

	void VulkanGeometryRenderPass::compile(FrameGraphResourcePool* resourcePool) {

		destroyTransientResources();

		createFramebuffers(resourcePool);
	}

	void VulkanGeometryRenderPass::execute(Scene* scene) {

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

	void VulkanGeometryRenderPass::buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		const Viewport& sceneViewport = scene->viewport();

		Entity cameraEntity = scene->cameraEntity();

		if(!cameraEntity.valid()) {
			// TODO
			throw MILO_RUNTIME_EXCEPTION("Main Camera Entity is not valid");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass;
			renderPassInfo.framebuffer = m_Framebuffers[imageIndex];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent.width = sceneViewport.width;
			renderPassInfo.renderArea.extent.height = sceneViewport.height;

			VkClearValue clearValues[2];
			clearValues[0].color = {0.01f, 0.01f, 0.01f, 1};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues;

			VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
			{
				VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

				const Viewport& sceneViewport = scene->viewport();

				VkViewport viewport{};
				viewport.x = (float)sceneViewport.x;
				viewport.y = (float)sceneViewport.y;
				viewport.width = (float)sceneViewport.width;
				viewport.height = (float)sceneViewport.height;
				viewport.minDepth = 0;
				viewport.maxDepth = 1;

				VkRect2D scissor{};
				scissor.offset = {0, 0};
				scissor.extent = {(uint32_t)viewport.width, (uint32_t)viewport.height};

				VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
				VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

				Camera& camera = cameraEntity.getComponent<Camera>();

				CameraData cameraData{};
				cameraData.proj = camera.projectionMatrix();
				cameraData.view = camera.viewMatrix(cameraEntity.getComponent<Transform>().translation);
				cameraData.projView = cameraData.proj * cameraData.view;

				m_CameraUniformBuffer->update(imageIndex, cameraData);

				VkDescriptorSet cameraDescriptorSet = m_CameraDescriptorPool->get(imageIndex);

				uint32_t dynamicOffsets[] = {imageIndex * m_CameraUniformBuffer->elementSize(), 0};

				auto& materialResources = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

				Mesh* lastMesh = nullptr;
				Material* lastMaterial = nullptr;

				auto components = scene->group<Transform, MeshView>();
				for(EntityId entityId : components) {

					const Transform& transform = components.get<Transform>(entityId);
					const MeshView& meshView = components.get<MeshView>(entityId);

					if(lastMaterial != meshView.material) {

						VkDescriptorSet materialDescriptorSet = materialResources.descriptorSetOf(meshView.material, dynamicOffsets[1]);
						VkDescriptorSet descriptorSets[] = {cameraDescriptorSet, materialDescriptorSet};

						VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
														 m_GraphicsPipeline->pipelineLayout(),
														 0, 2, descriptorSets, 2, dynamicOffsets));

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
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		}
		VK_CALLV(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanGeometryRenderPass::createRenderPass() {

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

	void VulkanGeometryRenderPass::createFramebuffers(FrameGraphResourcePool* resourcePool) {

		VulkanFrameGraphResourcePool* pool = dynamic_cast<VulkanFrameGraphResourcePool*>(resourcePool);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {

			const RenderTarget& renderTarget = pool->getRenderTarget(i);

			const VulkanTexture2D* colorTexture = dynamic_cast<const VulkanTexture2D*>(renderTarget.colorAttachment);
			const VulkanTexture2D* depthTexture = dynamic_cast<const VulkanTexture2D*>(renderTarget.depthAttachment);

			VkImageView attachments[2] = { colorTexture->vkImageView(), depthTexture->vkImageView() };

			VkFramebufferCreateInfo createInfo = mvk::FramebufferCreateInfo::create(m_RenderPass,
																					colorTexture->width(), colorTexture->height());
			createInfo.pAttachments = attachments;
			createInfo.attachmentCount = 2;

			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &createInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanGeometryRenderPass::createCameraUniformBuffer() {
		m_CameraUniformBuffer = new VulkanUniformBuffer<CameraData>();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanGeometryRenderPass::createCameraDescriptorLayout() {

		Array<VkDescriptorSetLayoutBinding, 1> bindings{};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = bindings.data();
		createInfo.bindingCount = bindings.size();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_CameraDescriptorSetLayout));
	}

	void VulkanGeometryRenderPass::createCameraDescriptorPool() {

		VulkanDescriptorPool::CreateInfo createInfo = {};
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.layout = m_CameraDescriptorSetLayout;

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSize.descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.poolSizes.push_back(poolSize);

		m_CameraDescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanGeometryRenderPass::createCameraDescriptorSets() {

		m_CameraDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_CameraUniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(CameraData);

			VkWriteDescriptorSet writeDescriptorSet = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptorSet, 0, nullptr));
		});
	}

	void VulkanGeometryRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		VkPushConstantRange pushConstants = {};
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		auto& materials = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		pipelineInfo.pushConstantRanges.push_back(pushConstants);
		pipelineInfo.setLayouts.push_back(m_CameraDescriptorSetLayout);
		pipelineInfo.setLayouts.push_back(materials.materialDescriptorSetLayout());

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/geometry/geometry.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/geometry/geometry.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanGeometryRenderPass", m_Device, pipelineInfo);
	}

	void VulkanGeometryRenderPass::createCommandPool() {
		m_CommandPool = new VulkanCommandPool(m_Device->graphicsQueue(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	}

	void VulkanGeometryRenderPass::createCommandBuffers() {
		m_CommandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_CommandBuffers);
	}

	void VulkanGeometryRenderPass::createSemaphores() {

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}
}
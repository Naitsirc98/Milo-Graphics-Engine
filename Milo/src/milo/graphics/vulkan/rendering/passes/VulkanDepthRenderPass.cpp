#include "milo/graphics/vulkan/rendering/passes/VulkanDepthRenderPass.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"

namespace milo {

	VulkanDepthRenderPass::VulkanDepthRenderPass() {
		m_Device = VulkanContext::get()->device();
		createRenderPass();
		createUniformBuffer();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
		createSemaphores();
	}

	VulkanDepthRenderPass::~VulkanDepthRenderPass() {
		m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
		DELETE_PTR(m_GraphicsPipeline);
		DELETE_PTR(m_UniformBuffer);
		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(m_Device->logical(), m_DescriptorSetLayout, nullptr));
		VK_CALLV(vkDestroyRenderPass(m_Device->logical(), m_RenderPass, nullptr));
		for(VkSemaphore semaphore : m_SignalSemaphores) {
			VK_CALLV(vkDestroySemaphore(m_Device->logical(), semaphore, nullptr));
		}
	}

	bool VulkanDepthRenderPass::shouldCompile(Scene* scene) const {
		return WorldRenderer::get().getFramebuffer().size() != m_LastFramebufferSize;
	}

	void VulkanDepthRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {

		m_LastFramebufferSize = WorldRenderer::get().getFramebuffer().size();

		if(m_GraphicsPipeline != nullptr) {
			DELETE_PTR(m_GraphicsPipeline);
			createGraphicsPipeline();
		}

		if(m_CommandBuffers[0] != VK_NULL_HANDLE) {
			m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
			m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
		}
	}

	void VulkanDepthRenderPass::execute(Scene* scene) {

		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* queue = m_Device->graphicsQueue();

		buildCommandBuffers(imageIndex, commandBuffer, scene);

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

	void VulkanDepthRenderPass::buildCommandBuffers(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		const Viewport& sceneViewport = scene->viewport();

		Entity cameraEntity = scene->cameraEntity();

		auto& framebuffer = dynamic_cast<VulkanFramebuffer&>(WorldRenderer::get().getFramebuffer());
		VkFramebuffer vkFramebuffer = framebuffer.get(m_RenderPass);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass;
			renderPassInfo.framebuffer = vkFramebuffer;
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent.width = framebuffer.size().width;
			renderPassInfo.renderArea.extent.height = framebuffer.size().height;

			VkClearValue clearValues[2];
			clearValues[0].color = {0.01f, 0.01f, 0.01f, 1};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues;

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
				scissor.extent = {(uint32_t)viewport.width, (uint32_t)viewport.height};

				VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
				VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

				if(cameraEntity.valid()) {
					renderMeshViews(imageIndex, commandBuffer, scene, cameraEntity);
				}
			}
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		}
		VK_CALLV(vkEndCommandBuffer(commandBuffer));
	}

	void VulkanDepthRenderPass::renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene, Entity cameraEntity) {

		/*

		CameraData cameraData{};

		if(getSimulationState() == SimulationState::Editor) {

			EditorCamera& camera = MiloEditor::camera();
			cameraData.proj = camera.projMatrix();
			cameraData.view = camera.viewMatrix();
			cameraData.projView = cameraData.proj * cameraData.view;

		} else {

			Camera& camera = cameraEntity.getComponent<Camera>();
			cameraData.proj = camera.projectionMatrix();
			cameraData.view = camera.viewMatrix(cameraEntity.getComponent<Transform>().translation);
			cameraData.projView = cameraData.proj * cameraData.view;
		}

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

			if(meshView.mesh == nullptr || meshView.material == nullptr) continue;

			if(lastMaterial != meshView.material) {

				VkDescriptorSet materialDescriptorSet = materialResources.descriptorSetOf(meshView.material, dynamicOffsets[1]);
				VkDescriptorSet descriptorSets[] = {cameraDescriptorSet, materialDescriptorSet};

				VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
												 m_GraphicsPipeline->pipelineLayout(),
												 0, 2, descriptorSets, 2, dynamicOffsets));

				lastMaterial = meshView.material;
			}

			if(lastMesh != meshView.mesh) {

				auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(meshView.mesh->buffers());

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
		 */
	}

	void VulkanDepthRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Load});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, RenderPass::LoadOp::Load};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanDepthRenderPass::createDescriptorSetLayout() {

		VkDescriptorSetLayoutBinding binding{};
		// Uniform buffer
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = &binding;
		createInfo.bindingCount = 1;

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_DescriptorSetLayout));
	}

	void VulkanDepthRenderPass::createDescriptorPool() {

		VkDescriptorPoolSize poolSize{};
		// Uniform buffer
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSize.descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.poolSizes.push_back(poolSize);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanDepthRenderPass::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<UniformBuffer>();
		m_UniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanDepthRenderPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = index * m_UniformBuffer->elementSize();
			bufferInfo.range = sizeof(UniformBuffer);

			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanDepthRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;
		pipelineInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		pipelineInfo.shaders.push_back({"resources/shaders/shadows/shadow_map.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/shadows/shadow_map.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanDepthRenderPass", m_Device, pipelineInfo);
	}

	void VulkanDepthRenderPass::createSemaphores() {

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}
}
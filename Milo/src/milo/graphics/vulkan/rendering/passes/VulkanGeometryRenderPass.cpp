#include "milo/graphics/vulkan/rendering/passes/VulkanGeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/scenes/Entity.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/editor/MiloEditor.h"

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

	bool VulkanGeometryRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}

	void VulkanGeometryRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {

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

		mvk::CommandBuffer::BeginGraphicsRenderPassInfo beginInfo{};
		beginInfo.renderPass = m_RenderPass;
		beginInfo.graphicsPipeline = m_GraphicsPipeline->vkPipeline();

		VkClearValue clearValues[2];
		clearValues[0].color = {0, 0, 0, 0};
		clearValues[1].depthStencil = {1, 0};

		beginInfo.clearValues = clearValues;
		beginInfo.clearValuesCount = 2;

		mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);

		renderScene(imageIndex, commandBuffer, scene);

		mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
	}

	void VulkanGeometryRenderPass::renderScene(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		CameraData cameraData{};

		if(getSimulationState() == SimulationState::Editor) {

			EditorCamera& camera = MiloEditor::camera();
			cameraData.proj = camera.projMatrix();
			cameraData.view = camera.viewMatrix();
			cameraData.projView = cameraData.proj * cameraData.view;

		} else {

			Entity cameraEntity = scene->cameraEntity();
			Camera& camera = cameraEntity.getComponent<Camera>();
			cameraData.proj = camera.projectionMatrix();
			cameraData.view = camera.viewMatrix(cameraEntity.getComponent<Transform>().translation());
			cameraData.projView = cameraData.proj * cameraData.view;
		}

		m_CameraUniformBuffer->update(imageIndex, cameraData);

		VkDescriptorSet cameraDescriptorSet = m_CameraDescriptorPool->get(imageIndex);

		uint32_t dynamicOffsets[] = {imageIndex * m_CameraUniformBuffer->elementSize(), 0};

		auto& materialResources = dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

		Mesh* lastMesh = nullptr;
		Material* lastMaterial = nullptr;

		for(const DrawCommand& command : WorldRenderer::get().drawCommands()) {

			if(lastMaterial != command.material) {

				VkDescriptorSet materialDescriptorSet = materialResources.descriptorSetOf(command.material, dynamicOffsets[1]);
				VkDescriptorSet descriptorSets[] = {cameraDescriptorSet, materialDescriptorSet};

				VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
												 m_GraphicsPipeline->pipelineLayout(),
												 0, 2, descriptorSets, 2, dynamicOffsets));

				lastMaterial = command.material;
			}

			if(lastMesh != command.mesh) {

				auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(command.mesh->buffers());

				VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
				VkDeviceSize offsets[] = {0};
				VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

				if(!command.mesh->indices().empty()) {
					VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
				}

				lastMesh = command.mesh;
			}

			VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT,
										0, sizeof(Matrix4), &command.transform));

			if(command.mesh->indices().empty()) {
				VK_CALLV(vkCmdDraw(commandBuffer, command.mesh->vertices().size(), 1, 0, 0));
			} else {
				VK_CALLV(vkCmdDrawIndexed(commandBuffer, command.mesh->indices().size(), 1, 0, 0, 0));
			}
		}
	}

	void VulkanGeometryRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Clear});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanGeometryRenderPass::createCameraUniformBuffer() {
		m_CameraUniformBuffer = VulkanUniformBuffer<CameraData>::create();
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

		//pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

		pipelineInfo.shaders.push_back({"resources/shaders/geometry/geometry.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/geometry/geometry.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

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
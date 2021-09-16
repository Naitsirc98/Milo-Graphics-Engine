#include <milo/graphics/vulkan/buffers/VulkanFramebuffer.h>
#include "milo/graphics/vulkan/rendering/passes/VulkanGridRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/editor/MiloEditor.h"

namespace milo {

	static const Matrix4 TRANSFORM = scale(Matrix4(1.0f), Vector3(8.0f));

	VulkanGridRenderPass::VulkanGridRenderPass() {
		m_Device = VulkanContext::get()->device();
		createRenderPass();
		createUniformBuffer();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
		createSemaphores();
	}

	VulkanGridRenderPass::~VulkanGridRenderPass() {
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

	bool VulkanGridRenderPass::shouldCompile(Scene* scene) const {
		return WorldRenderer::get().getFramebuffer().size() != m_LastFramebufferSize;
	}

	void VulkanGridRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {

		if(m_GraphicsPipeline != nullptr) {
			DELETE_PTR(m_GraphicsPipeline);
		}
		createGraphicsPipeline();

		if(m_CommandBuffers[0] != VK_NULL_HANDLE) {
			m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
		}
		createCommandBuffers(resourcePool);

		m_LastFramebufferSize = WorldRenderer::get().getFramebuffer().size();
	}

	void VulkanGridRenderPass::execute(Scene* scene) {

		SkyboxView* skyboxView = scene->skyboxView();
		if(skyboxView == nullptr) return;

		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* queue = m_Device->graphicsQueue();

		UniformBuffer uniformBufferData{};

		if(getSimulationState() == SimulationState::Editor) {

			EditorCamera& camera = MiloEditor::camera();
			Matrix4 modelMatrix = translate(Matrix4(1.0f), camera.position() - Vector3(0, 1, 0)) * scale(Matrix4(1.0f), Vector3(10.0f));
			uniformBufferData.projViewModel = camera.projMatrix() * camera.viewMatrix() * modelMatrix;

		} else {

			Entity cameraEntity = scene->cameraEntity();
			Camera& camera = cameraEntity.getComponent<Camera>();
			uniformBufferData.projViewModel = camera.projectionMatrix()
					* camera.viewMatrix(cameraEntity.getComponent<Transform>().translation())
					* TRANSFORM;
		}

		uniformBufferData.scale = 16;//16.025f;
		uniformBufferData.size  = 0.05f;//0.025f;

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

	void VulkanGridRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Load});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, LoadOp::Load};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanGridRenderPass::createDescriptorSetLayout() {

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

	void VulkanGridRenderPass::createDescriptorPool() {

		VkDescriptorPoolSize poolSizes{};
		// Uniform buffer
		poolSizes.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes.descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_DescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.poolSizes.push_back(poolSizes);

		m_DescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanGridRenderPass::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<UniformBuffer>();
		m_UniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanGridRenderPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBuffer);

			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(
					0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanGridRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaders.push_back({"resources/shaders/grid/grid.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/grid/grid.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanGridRenderPass", m_Device, pipelineInfo);
	}

	void VulkanGridRenderPass::createCommandBuffers(FrameGraphResourcePool* resourcePool) {


		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY,
												  m_CommandBuffers.size(), m_CommandBuffers.data());

		Scene* scene = SceneManager::activeScene();

		Mesh* mesh = Assets::meshes().getPlane();

		auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());

		for(uint32_t imageIndex = 0; imageIndex < m_CommandBuffers.size(); ++imageIndex) {

			VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			const Viewport& sceneViewport = scene->viewport();

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
			{
				auto framebuffer = dynamic_cast<VulkanFramebuffer*>(resourcePool->getDefaultFramebuffer(imageIndex));

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_RenderPass;
				renderPassInfo.framebuffer = framebuffer->get(m_RenderPass);
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent.width = framebuffer->size().width;
				renderPassInfo.renderArea.extent.height = framebuffer->size().height;

				VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
				{
					VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

					VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);

					uint32_t dynamicOffset = m_UniformBuffer->elementSize() * imageIndex;

					VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
													 m_GraphicsPipeline->pipelineLayout(),
													 0, 1, &descriptorSet, 1, &dynamicOffset));

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

	void VulkanGridRenderPass::createSemaphores() {

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}

}
#include "milo/graphics/vulkan/rendering/passes/VulkanPreDepthRenderPass.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	VulkanPreDepthRenderPass::VulkanPreDepthRenderPass() {
		m_Device = VulkanContext::get()->device();
		createRenderPass();
		createUniformBuffer();
		createDescriptorSetLayout();
		createDescriptorPool();
		createDescriptorSets();
		createSemaphores();
	}

	VulkanPreDepthRenderPass::~VulkanPreDepthRenderPass() {
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

	bool VulkanPreDepthRenderPass::shouldCompile(Scene* scene) const {
		return WorldRenderer::get().getFramebuffer().size() != m_LastFramebufferSize;
	}

	void VulkanPreDepthRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {

		m_LastFramebufferSize = WorldRenderer::get().getFramebuffer().size();

		createFramebuffers(m_LastFramebufferSize, resourcePool);

		if(m_GraphicsPipeline != nullptr) {
			DELETE_PTR(m_GraphicsPipeline);
		}
		createGraphicsPipeline();

		if(m_CommandBuffers[0] != VK_NULL_HANDLE) {
			m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
		}
		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	void VulkanPreDepthRenderPass::execute(Scene* scene) {

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

	void VulkanPreDepthRenderPass::buildCommandBuffers(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		mvk::CommandBuffer::BeginGraphicsRenderPassInfo beginInfo{};
		beginInfo.renderPass = m_RenderPass;
		beginInfo.graphicsPipeline = m_GraphicsPipeline->vkPipeline();
		beginInfo.framebuffer = m_Framebuffers[imageIndex].get();

		VkClearValue clearValues[2];
		clearValues[0].color = {0, 0, 0, 0};
		clearValues[1].depthStencil = {1, 0};

		beginInfo.clearValues = clearValues;
		beginInfo.clearValuesCount = 2;

		mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);
		renderMeshViews(imageIndex, commandBuffer, scene);
		mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
	}

	void VulkanPreDepthRenderPass::renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		{
			const auto& camera = WorldRenderer::get().camera();
			UniformBuffer ubo{camera.proj, camera.view, camera.projView};
			m_UniformBuffer->update(imageIndex, ubo);
		}

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);
		uint32_t dynamicOffset[1] = {m_UniformBuffer->elementSize() * imageIndex};
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout(),
										 0, 1, &descriptorSet, 1, dynamicOffset));

		Mesh* lastMesh = nullptr;

		for(const DrawCommand& command : WorldRenderer::get().drawCommands()) {

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

			VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(),
										VK_SHADER_STAGE_VERTEX_BIT,
										0, sizeof(Matrix4), &command.transform));

			if(command.mesh->indices().empty()) {
				VK_CALLV(vkCmdDraw(commandBuffer, command.mesh->vertices().size(), 1, 0, 0));
			} else {
				VK_CALLV(vkCmdDrawIndexed(commandBuffer, command.mesh->indices().size(), 1, 0, 0, 0));
			}
		}
	}

	void VulkanPreDepthRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::R32F, 1, RenderPass::LoadOp::Clear});
		desc.depthAttachment = {PixelFormat::DEPTH32, 1, RenderPass::LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanPreDepthRenderPass::createDescriptorSetLayout() {

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

	void VulkanPreDepthRenderPass::createDescriptorPool() {

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

	void VulkanPreDepthRenderPass::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<UniformBuffer>();
		m_UniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanPreDepthRenderPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBuffer);

			VkWriteDescriptorSet writeDescriptor = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanPreDepthRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaders.push_back({"resources/shaders/pre_depth/pre_depth.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/pre_depth/pre_depth.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(Matrix4);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		pipelineInfo.pushConstantRanges.push_back(pushConstant);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanPreDepthRenderPass", m_Device, pipelineInfo);
	}

	void VulkanPreDepthRenderPass::createSemaphores() {
		mvk::Semaphore::create(m_SignalSemaphores.size(), m_SignalSemaphores.data());
	}

	void VulkanPreDepthRenderPass::createFramebuffers(const Size& size, FrameGraphResourcePool* resourcePool) {

		VulkanFramebuffer::ApiInfo apiInfo = {m_Device};

		Framebuffer::CreateInfo createInfo{};
		createInfo.size = size;
		createInfo.colorAttachments.push_back(PixelFormat::R32F);
		createInfo.depthAttachments.push_back(PixelFormat::DEPTH32);
		createInfo.apiInfo = &apiInfo;

		for(uint32_t i = 0;i < m_Framebuffers.size();++i) {
			Handle handle = PreDepthRenderPass::createFramebufferHandle(i);
			if(m_Framebuffers[i] != nullptr) {
				resourcePool->removeFramebuffer(handle);
			}
			m_Framebuffers[i] = std::make_shared<VulkanFramebuffer>(createInfo);
			resourcePool->putFramebuffer(handle, m_Framebuffers[i]);
		}
	}
}
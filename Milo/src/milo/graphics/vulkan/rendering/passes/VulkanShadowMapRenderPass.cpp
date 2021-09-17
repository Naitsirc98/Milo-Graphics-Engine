#include "milo/graphics/vulkan/rendering/passes/VulkanShadowMapRenderPass.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/graphics/rendering/WorldRenderer.h"

namespace milo {

	VulkanShadowMapRenderPass::VulkanShadowMapRenderPass() {

		m_Device = VulkanContext::get()->device();

		createRenderPass();
		createUniformBuffer();
		createDescriptorSetLayoutAndPool();
		createDescriptorSets();
		createSemaphores();
		createShadowCascades();
		createGraphicsPipeline();
		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	VulkanShadowMapRenderPass::~VulkanShadowMapRenderPass() {
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

	bool VulkanShadowMapRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}

	void VulkanShadowMapRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {
	}

	void VulkanShadowMapRenderPass::execute(Scene* scene) {

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

	void VulkanShadowMapRenderPass::buildCommandBuffers(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		for(uint32_t cascadeIndex = 0;cascadeIndex < MAX_SHADOW_CASCADES;++cascadeIndex) {

			mvk::CommandBuffer::BeginGraphicsRenderPassInfo beginInfo{};
			beginInfo.renderPass = m_RenderPass;
			beginInfo.graphicsPipeline = m_GraphicsPipeline->vkPipeline();
			beginInfo.framebuffer = m_ShadowCascades[cascadeIndex][imageIndex].get();
			beginInfo.dynamicViewport = false;
			beginInfo.dynamicScissor = false;

			mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);
			renderMeshViews(imageIndex, commandBuffer, cascadeIndex);
			mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
		}
	}

	void VulkanShadowMapRenderPass::renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, uint32_t cascadeIndex) {
		bindDescriptorSets(imageIndex, commandBuffer);
		renderScene(commandBuffer, cascadeIndex);
	}

	void VulkanShadowMapRenderPass::bindDescriptorSets(uint32_t imageIndex, VkCommandBuffer commandBuffer) {

		{
			const auto& camera = WorldRenderer::get().camera();
			ShadowData shadows{camera.proj, camera.view, camera.projView};
			m_UniformBuffer->update(imageIndex, shadows);
		}

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(imageIndex);

		uint32_t dynamicOffset[1] = {m_UniformBuffer->elementSize() * imageIndex};
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										 m_GraphicsPipeline->pipelineLayout(),
										 0, 1, &descriptorSet, 1, dynamicOffset));
	}

	void VulkanShadowMapRenderPass::renderScene(VkCommandBuffer commandBuffer, uint32_t cascadeIndex) {

		Mesh* lastMesh = nullptr;

		for(const DrawCommand& command : WorldRenderer::get().drawCommands()) {

			if(lastMesh != command.mesh) {
				bindMesh(commandBuffer, command);
				lastMesh = command.mesh;
			}

			pushConstants(commandBuffer, cascadeIndex, command);

			draw(commandBuffer, command);
		}
	}

	void VulkanShadowMapRenderPass::draw(VkCommandBuffer commandBuffer, const DrawCommand& command) const {
		if(command.mesh->indices().empty()) {
			VK_CALLV(vkCmdDraw(commandBuffer, command.mesh->vertices().size(), 1, 0, 0));
		} else {
			VK_CALLV(vkCmdDrawIndexed(commandBuffer, command.mesh->indices().size(), 1, 0, 0, 0));
		}
	}

	void VulkanShadowMapRenderPass::pushConstants(VkCommandBuffer commandBuffer, uint32_t cascadeIndex,
												  const DrawCommand& command) const {
		PushConstants pushConstants{};
		pushConstants.modelMatrix = command.transform;
		pushConstants.cascadeIndex = cascadeIndex;
		VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(),
									VK_SHADER_STAGE_VERTEX_BIT,
									0, sizeof(PushConstants), &pushConstants));
	}

	void VulkanShadowMapRenderPass::bindMesh(VkCommandBuffer commandBuffer, const DrawCommand& command) const {

		auto* meshBuffers = dynamic_cast<VulkanMeshBuffers*>(command.mesh->buffers());

		VkBuffer vertexBuffers[] = {meshBuffers->vertexBuffer()->vkBuffer()};
		VkDeviceSize offsets[] = {0};
		VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

		if(!command.mesh->indices().empty()) {
			VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, meshBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
		}
	}

	void VulkanShadowMapRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.depthAttachment = {PixelFormat::DEPTH32, 1, RenderPass::LoadOp::Clear};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanShadowMapRenderPass::createDescriptorSetLayoutAndPool() {

		mvk::DescriptorSet::CreateInfo createInfo{};
		createInfo.numSets = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		createInfo.descriptors.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);

		m_DescriptorSetLayout = mvk::DescriptorSet::Layout::create(createInfo);
		m_DescriptorPool = mvk::DescriptorSet::Pool::create(m_DescriptorSetLayout, createInfo);
	}

	void VulkanShadowMapRenderPass::createUniformBuffer() {
		m_UniformBuffer = new VulkanUniformBuffer<ShadowData>();
		m_UniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanShadowMapRenderPass::createDescriptorSets() {

		m_DescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(ShadowData);

			auto writeDescriptor = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptor, 0, nullptr));
		});
	}

	void VulkanShadowMapRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_DescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaders.push_back({"resources/shaders/pre_depth/pre_depth.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/pre_depth/pre_depth.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		const Size& size = WorldRenderer::get().shadowsMapSize();

		pipelineInfo.viewport = {0, 0, (float)size.width, (float)size.height};
		pipelineInfo.scissor = {{0, 0}, {(uint32_t)size.width, (uint32_t)size.height}};

		VkPushConstantRange pushConstant;
		pushConstant.offset = 0;
		pushConstant.size = sizeof(PushConstants);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		pipelineInfo.pushConstantRanges.push_back(pushConstant);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanShadowMapRenderPass", m_Device, pipelineInfo);
	}

	void VulkanShadowMapRenderPass::createSemaphores() {
		mvk::Semaphore::create(m_SignalSemaphores.size(), m_SignalSemaphores.data());
	}

	void VulkanShadowMapRenderPass::createShadowCascades() {

		FrameGraphResourcePool& resourcePool = WorldRenderer::get().resources();

		VulkanFramebuffer::ApiInfo apiInfo = {};
		apiInfo.device = m_Device;

		VulkanFramebuffer::CreateInfo createInfo{};
		createInfo.size = WorldRenderer::get().shadowsMapSize();
		createInfo.depthAttachments.push_back(PixelFormat::DEPTH32);
		createInfo.apiInfo = &apiInfo;

		for(uint32_t i = 0;i < m_ShadowCascades.size();++i) {
			auto& cascade = m_ShadowCascades[i];
			for(uint32_t j = 0;j < cascade.size();++j) {
				Handle handle = ShadowMapRenderPass::getCascadeShadowMap(i, j);
				auto framebuffer = std::make_shared<VulkanFramebuffer>(createInfo);
				resourcePool.putFramebuffer(handle, framebuffer);
				cascade[j] = framebuffer;
			}
		}
	}
}
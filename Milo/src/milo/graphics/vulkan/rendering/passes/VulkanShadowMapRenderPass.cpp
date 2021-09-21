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
		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_PrimaryCommandBuffers.size(), m_PrimaryCommandBuffers.data());
	}

	VulkanShadowMapRenderPass::~VulkanShadowMapRenderPass() {
		m_Device->graphicsCommandPool()->free(m_PrimaryCommandBuffers.size(), m_PrimaryCommandBuffers.data());
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
		VkCommandBuffer commandBuffer = m_PrimaryCommandBuffers[imageIndex];
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

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_RenderPass;
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = {4096, 4096};

		VkClearValue clearValues[1];
		clearValues[0].depthStencil = {1, 0};

		renderPassInfo.pClearValues = clearValues;
		renderPassInfo.clearValueCount = 1;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			bindDescriptorSets(imageIndex, commandBuffer);

			VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

			renderShadowCascade(imageIndex, commandBuffer, 0, renderPassInfo);
			renderShadowCascade(imageIndex, commandBuffer, 1, renderPassInfo);
			renderShadowCascade(imageIndex, commandBuffer, 2, renderPassInfo);
			renderShadowCascade(imageIndex, commandBuffer, 3, renderPassInfo);
		}
		VK_CALLV(vkEndCommandBuffer(commandBuffer));
	}

	inline void VulkanShadowMapRenderPass::renderShadowCascade(uint32_t imageIndex, VkCommandBuffer commandBuffer,
															   uint32_t cascadeIndex, VkRenderPassBeginInfo& renderPassInfo) {


		renderPassInfo.framebuffer = m_ShadowCascades[cascadeIndex].framebuffer;

		VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));

		renderScene(commandBuffer, cascadeIndex);

		VK_CALLV(vkCmdEndRenderPass(commandBuffer));
	}

	inline void VulkanShadowMapRenderPass::bindDescriptorSets(uint32_t index, VkCommandBuffer commandBuffer) {

		{
			const auto& cascades = WorldRenderer::get().shadowCascades();
			ShadowData shadows = {
					cascades[0].viewProj,
					cascades[1].viewProj,
					cascades[2].viewProj,
					cascades[3].viewProj
			};
			m_UniformBuffer->update(index, shadows);
		}

		VkDescriptorSet descriptorSet = m_DescriptorPool->get(index);

		uint32_t dynamicOffset[1] = {m_UniformBuffer->elementSize() * index};
		VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
										 m_GraphicsPipeline->pipelineLayout(),
										 0, 1, &descriptorSet, 1, dynamicOffset));
	}

	void VulkanShadowMapRenderPass::renderScene(VkCommandBuffer commandBuffer, uint32_t cascadeIndex) {

		Mesh* lastMesh = nullptr;

		for(const DrawCommand& command : WorldRenderer::get().shadowsDrawCommands()) {

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

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = VK_FORMAT_D32_SFLOAT;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;
		subpass.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for layout transitions
		Array<VkSubpassDependency, 2> dependencies{};

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo renderPassCreateInfo{};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &depthAttachment;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassCreateInfo.pDependencies = dependencies.data();

		VK_CALL(vkCreateRenderPass(m_Device->logical(), &renderPassCreateInfo, nullptr, &m_RenderPass));
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
		m_UniformBuffer = VulkanUniformBuffer<ShadowData>::create();
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

		pipelineInfo.colorBlendAttachments.clear();

		//pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;

		pipelineInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		pipelineInfo.rasterizationState.depthClampEnable = VK_TRUE; // TODO: check for support

		pipelineInfo.shaders.push_back({"resources/shaders/shadows/shadow_map.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/shadows/shadow_map.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		//pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);

		const Size& size = WorldRenderer::get().shadowsMapSize();

		pipelineInfo.viewport = {0, 0, (float)size.width, (float)size.height, 0, 1};
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

		m_DepthTexture = Ref<VulkanTexture2DArray>(VulkanTexture2DArray::create(
				TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, MAX_SHADOW_CASCADES));

		VulkanTexture2DArray::AllocInfo allocInfo{};
		allocInfo.format = PixelFormat::DEPTH32;
		allocInfo.width = 4096;
		allocInfo.height = 4096;
		allocInfo.mipLevels = 1;

		m_DepthTexture->allocate(allocInfo);

		VkSamplerCreateInfo sampler = mvk::SamplerCreateInfo::create();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		m_DepthTexture->vkSampler(VulkanContext::get()->samplerMap()->get(sampler));

		resourcePool.putTexture2D(ShadowMapRenderPass::getDepthMap(0), m_DepthTexture); // TODO

		for(uint32_t i = 0;i < m_ShadowCascades.size();++i) {

			VulkanShadowCascade& cascade = m_ShadowCascades[i];

			cascade.imageView = m_DepthTexture->getLayer(i);

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.pAttachments = &cascade.imageView;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.width = 4096;
			framebufferInfo.height = 4096;
			framebufferInfo.layers = 1;

			VK_CALLV(vkCreateFramebuffer(m_Device->logical(), &framebufferInfo, nullptr, &cascade.framebuffer));
		}
	}

	VulkanShadowCascade::~VulkanShadowCascade() {
		VkDevice device = VulkanContext::get()->device()->logical();
		VK_CALLV(vkDestroyFramebuffer(device, framebuffer, nullptr));
	}
}
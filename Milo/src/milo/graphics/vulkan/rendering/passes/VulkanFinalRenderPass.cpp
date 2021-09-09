#include "milo/graphics/vulkan/rendering/passes/VulkanFinalRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/WorldRenderer.h"

namespace milo {

	VulkanFinalRenderPass::VulkanFinalRenderPass() {

		m_Device = VulkanContext::get()->device();
		m_Swapchain = VulkanContext::get()->swapchain();

		createRenderPass();

		createSemaphores();
	}

	VulkanFinalRenderPass::~VulkanFinalRenderPass() {

		destroyTransientResources();

		for(VkSemaphore semaphore : m_SignalSemaphores) {
			VK_CALLV(vkDestroySemaphore(m_Device->logical(), semaphore, nullptr));
		}

		DELETE_PTR(m_CommandPool);

		VK_CALLV(vkDestroyRenderPass(m_Device->logical(), m_RenderPass, nullptr));
	}

	bool VulkanFinalRenderPass::shouldCompile(Scene* scene) const {
		return m_Swapchain->size() != m_LastSwapchainSize
			|| WorldRenderer::get().getFramebuffer().size() != m_LastFramebufferSize;
	}

	void VulkanFinalRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {

		m_LastSwapchainSize = m_Device->context()->swapchain()->size();
		m_LastFramebufferSize = WorldRenderer::get().getFramebuffer().size();

		destroyTransientResources();

		createFramebuffers(resourcePool);

		createTextureDescriptorLayout();
		createTextureDescriptorPool();
		createTextureDescriptorSets(resourcePool);

		createGraphicsPipeline();

		createCommandPool();
		createCommandBuffers(resourcePool);
	}

	void VulkanFinalRenderPass::execute(Scene* scene) {

		VulkanQueue* queue = m_Device->graphicsQueue();
		VulkanPresenter* presenter = m_Device->context()->vulkanPresenter();
		uint32_t imageIndex = presenter->currentImageIndex();
		//VkFence frameFence = presenter->frameInFlightFence();
		//queue->waitSemaphores().push_back(presenter->imageAvailableSemaphore());
		//VkSemaphore renderFinishedSemaphore = presenter->renderFinishedSemaphore();

		// This render pass have to wait for:
		// 1: offscreen rendering pass to complete to sample the resulting texture
		// 2: swapchain image to render to

		VkPipelineStageFlags waitStages[] = {
				//VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.pSignalSemaphores = &m_SignalSemaphores[imageIndex];//&renderFinishedSemaphore;
		submitInfo.signalSemaphoreCount = 1;

		queue->submit(submitInfo, VK_NULL_HANDLE);
	}

	void VulkanFinalRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createPresentSrcAttachment();
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = &attachmentRef;
		subpass.colorAttachmentCount = 1;

		VkAttachmentDescription attachments[] = {colorAttachment};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.subpassCount = 1;

		VK_CALL(vkCreateRenderPass(m_Device->logical(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void VulkanFinalRenderPass::createFramebuffers(FrameGraphResourcePool* pool) {

		uint32_t imageCount = m_Swapchain->imageCount();
		const VulkanSwapchainImage* swapchainImages = m_Swapchain->images();

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = m_RenderPass;
		createInfo.layers = 1;
		createInfo.width = m_Swapchain->extent().width;
		createInfo.height = m_Swapchain->extent().height;
		createInfo.attachmentCount = 1;

		for(uint32_t i = 0;i < imageCount;++i) {

			auto image = swapchainImages[i];

			createInfo.pAttachments = &image.vkImageView;

			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &createInfo, nullptr, &m_Framebuffers[i]));
		}
	}

	void VulkanFinalRenderPass::createTextureDescriptorLayout() {

		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.pBindings = &binding;
		createInfo.bindingCount = 1;

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_TextureDescriptorSetLayout));
	}

	void VulkanFinalRenderPass::createTextureDescriptorPool() {

		VulkanDescriptorPool::CreateInfo createInfo{};
		createInfo.layout = m_TextureDescriptorSetLayout;
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.poolSizes.push_back(poolSize);

		m_TextureDescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanFinalRenderPass::createTextureDescriptorSets(FrameGraphResourcePool* resourcePool) {

		VkSampler sampler = VulkanContext::get()->samplerMap()->getDefaultSampler();

		m_TextureDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			auto texture = dynamic_cast<VulkanTexture2D*>(resourcePool->getDefaultFramebuffer(index)->colorAttachments()[0]);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->vkImageView();
			imageInfo.sampler = sampler;

			VkWriteDescriptorSet writeDescriptorSet = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(
					0,
					descriptorSet,
					1,
					&imageInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1,
											&writeDescriptorSet, 0, nullptr));
		});
	}

	void VulkanFinalRenderPass::createGraphicsPipeline() {

		const Viewport& viewport = SceneManager::activeScene()->viewport();

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.setLayouts.push_back(m_TextureDescriptorSetLayout);

		pipelineInfo.depthStencil.depthTestEnable = VK_FALSE;

		pipelineInfo.viewport = {viewport.x, viewport.y, viewport.width, viewport.height};
		pipelineInfo.scissor = {{(int32_t)viewport.x, (int32_t)viewport.y}, {(uint32_t)viewport.width, (uint32_t)viewport.height}};

		pipelineInfo.shaders.push_back({Files::resource("shaders/fullscreen_quad/fullscreen_quad.vert"), VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({Files::resource("shaders/fullscreen_quad/fullscreen_quad.frag"), VK_SHADER_STAGE_FRAGMENT_BIT});

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanFinalRenderPass", m_Device, pipelineInfo);
	}

	void VulkanFinalRenderPass::createCommandPool() {
		m_CommandPool = new VulkanCommandPool(m_Device->graphicsQueue());
	}

	void VulkanFinalRenderPass::createCommandBuffers(FrameGraphResourcePool* resourcePool) {

		VulkanSwapchain* swapchain = m_Device->context()->swapchain();
		VkDescriptorSet* descriptorSets = m_TextureDescriptorPool->descriptorSets();
		Mesh* mesh = Assets::meshes().getQuad();
		auto buffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());
		VkBuffer vertexBuffer = buffers->vertexBuffer()->vkBuffer();
		VkBuffer indexBuffer = mesh->indices().empty() ? VK_NULL_HANDLE : buffers->indexBuffer()->vkBuffer();

		VulkanFrameGraphResourcePool* pool = dynamic_cast<VulkanFrameGraphResourcePool*>(resourcePool);

		m_CommandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_CommandBuffers);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {

			VkCommandBuffer commandBuffer = m_CommandBuffers[i];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
			{
				VkClearValue clearValues{};
				clearValues.color = {1, 0, 0, 1};

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_RenderPass;
				renderPassInfo.framebuffer = m_Framebuffers[i];
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapchain->extent();
				renderPassInfo.pClearValues = &clearValues;
				renderPassInfo.clearValueCount = 1;

				auto texture = dynamic_cast<VulkanTexture2D*>(pool->getDefaultFramebuffer(i)->colorAttachments()[0]);

				texture->setLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
				{
					VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

					VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
													 m_GraphicsPipeline->pipelineLayout(), 0, 1, descriptorSets, 0, nullptr));

					VkDeviceSize offsets = 0;
					VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offsets));

					if(!mesh->indices().empty()) {
						VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32));
					}

					if(mesh->indices().empty()) {
						VK_CALLV(vkCmdDraw(commandBuffer, mesh->vertices().size(), 1, 0, 0));
					} else {
						VK_CALLV(vkCmdDrawIndexed(commandBuffer, mesh->indices().size(), 1, 0, 0, 0));
					}
				}
				VK_CALLV(vkCmdEndRenderPass(commandBuffer));
			}
			VK_CALL(vkEndCommandBuffer(commandBuffer));
		}
	}

	void VulkanFinalRenderPass::createSemaphores() {

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for(uint32_t i = 0;i < m_SignalSemaphores.size();++i) {
			VK_CALL(vkCreateSemaphore(m_Device->logical(), &createInfo, nullptr, &m_SignalSemaphores[i]));
		}
	}

	void VulkanFinalRenderPass::destroyTransientResources() {

		m_Device->awaitTermination();

		VkDevice device = m_Device->logical();

		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_TextureDescriptorSetLayout, nullptr));
		DELETE_PTR(m_TextureDescriptorPool);

		DELETE_PTR(m_GraphicsPipeline);

		DELETE_PTR(m_CommandPool);

		for(VkFramebuffer& framebuffer : m_Framebuffers) {
			if(framebuffer == VK_NULL_HANDLE) continue;
			VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), framebuffer, nullptr));
			framebuffer = VK_NULL_HANDLE;
		}
	}
}
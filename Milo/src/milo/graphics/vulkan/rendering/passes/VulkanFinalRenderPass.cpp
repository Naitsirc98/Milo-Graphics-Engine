#include "milo/graphics/vulkan/rendering/passes/VulkanFinalRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/assets/AssetManager.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	VulkanFinalRenderPass::VulkanFinalRenderPass() {
		m_Device = VulkanContext::get()->device();
	}

	VulkanFinalRenderPass::~VulkanFinalRenderPass() {
		// TODO
	}

	void VulkanFinalRenderPass::compile(FrameGraphResourcePool* resourcePool) {

		// TODO: destroy recreated resources

		if(m_RenderPass == VK_NULL_HANDLE) {
			createRenderPass();
		}

		createFramebuffers();

		createTextureDescriptorLayout();
		createTextureDescriptorPool();
		createTextureDescriptorSets(resourcePool);

		createPipelineLayout();
		createGraphicsPipeline();

		createCommandPool();
		createCommandBuffers();
	}

	void VulkanFinalRenderPass::execute(Scene* scene) {
		// TODO
	}

	void VulkanFinalRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createPresentSrcAttachment();

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

	void VulkanFinalRenderPass::createFramebuffers() {

		VulkanSwapchain* swapchain = m_Device->context()->swapchain();

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.layers = 1;
		framebufferInfo.width = swapchain->extent().width;
		framebufferInfo.height = swapchain->extent().height;

		const VulkanSwapchainImage* swapchainImages = swapchain->images();
		
		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			const VulkanSwapchainImage& colorTexture = swapchainImages[i];
			VkImageView attachments[] = {colorTexture.vkImageView};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
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

		auto resPool = dynamic_cast<VulkanFrameGraphResourcePool*>(resourcePool);

		auto& textures = resPool->getTextures2D(m_Input.textures[0].handle);

		VkSampler sampler = VulkanContext::get()->samplerMap()->getDefaultSampler();

		m_TextureDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VulkanTexture2D* texture = dynamic_cast<VulkanTexture2D*>(textures[index].texture);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->vkImageView();
			imageInfo.sampler = sampler;

			VkWriteDescriptorSet writeDescriptorSet = mvk::WriteDescriptorSet::createCombineImageSamplerWrite(0, descriptorSet, 1, &imageInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptorSet, 0, nullptr));
		});
	}

	void VulkanFinalRenderPass::createPipelineLayout() {

		VkPipelineLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pSetLayouts = &m_TextureDescriptorSetLayout;
		createInfo.setLayoutCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanFinalRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipelineInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;
		pipelineInfo.vkPipelineLayout = m_PipelineLayout;
		pipelineInfo.vkPipelineCache = VK_NULL_HANDLE;

		pipelineInfo.depthStencil.depthTestEnable = VK_FALSE;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/fullscreen_quad/fullscreen_quad.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/fullscreen_quad/fullscreen_quad.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_GraphicsPipeline = VulkanGraphicsPipeline::create("VulkanFinalRenderPass", m_Device->logical(), pipelineInfo);
	}

	void VulkanFinalRenderPass::createCommandPool() {
		m_CommandPool = new VulkanCommandPool(m_Device->graphicsQueue());
	}

	void VulkanFinalRenderPass::createCommandBuffers() {

		VulkanSwapchain* swapchain = m_Device->context()->swapchain();
		VkDescriptorSet* descriptorSets = m_TextureDescriptorPool->descriptorSets();
		Mesh* mesh = Assets::meshes().getPlane();
		auto buffers = dynamic_cast<VulkanMeshBuffers*>(mesh->buffers());
		VkBuffer vertexBuffer = buffers->vertexBuffer()->vkBuffer();

		m_CommandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_CommandBuffers);

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {

			VkCommandBuffer commandBuffer = m_CommandBuffers[i];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
			{
				VkClearValue clearValues{};
				clearValues.color = {{0.1f, 0.1f, 0.1f, 1.0f}};

				VkRenderPassBeginInfo renderPassInfo = {};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = m_RenderPass;
				renderPassInfo.framebuffer = m_Framebuffers[i];
				renderPassInfo.renderArea.offset = {0, 0};
				renderPassInfo.renderArea.extent = swapchain->extent();
				renderPassInfo.pClearValues = &clearValues;
				renderPassInfo.clearValueCount = 1;

				VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
				{
					VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline));

					VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
													 m_PipelineLayout, 0, 1, descriptorSets, 0, nullptr));

					VkDeviceSize offsets = 0;
					VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offsets));

					VK_CALLV(vkCmdDraw(commandBuffer, mesh->vertices().size(), 1, 0, 0));
				}
				VK_CALLV(vkCmdEndRenderPass(commandBuffer));
			}
			VK_CALL(vkEndCommandBuffer(commandBuffer));
		}
	}
}
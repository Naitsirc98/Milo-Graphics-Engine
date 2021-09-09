#include "milo/graphics/vulkan/textures/VulkanIconFactory.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/materials/VulkanMaterialResourcePool.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	static const Color CLEAR_COLOR = {0.15f, 0.15f, 0.15f, 1.0f};

	VulkanIconFactory::VulkanIconFactory() {
		m_Device = VulkanContext::get()->device();
	}

	VulkanIconFactory::~VulkanIconFactory() {

		DELETE_PTR(m_GraphicsPipeline);
		DELETE_PTR(m_DepthTexture);

		vkDestroyRenderPass(m_Device->logical(), m_RenderPass, nullptr);
	}

	Texture2D* VulkanIconFactory::createIcon(Mesh* mesh, Material* material, const Size& size) {

		if(m_RenderPass == VK_NULL_HANDLE) {
			createRenderPass();
		}

		if(m_DepthTexture == nullptr) {
			createDepthTexture(DEFAULT_ICON_SIZE);
		} else if(m_DepthTexture->size() != size) {
			m_DepthTexture->resize(size);
		}

		if(m_GraphicsPipeline == nullptr) {
			createGraphicsPipeline();
		}

		VulkanTexture2D* iconTexture = createTexture2D(size);

		VkFramebuffer framebuffer = createFramebuffer(iconTexture, size);

		m_Device->graphicsCommandPool()->execute([&](VkCommandBuffer commandBuffer) {

			VkClearValue clearValues[2];
			clearValues[0].color = {CLEAR_COLOR.r, CLEAR_COLOR.g, CLEAR_COLOR.b, CLEAR_COLOR.a};
			clearValues[1].depthStencil = {1, 0};

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_RenderPass;
			renderPassInfo.framebuffer = framebuffer;
			renderPassInfo.pClearValues = clearValues;
			renderPassInfo.clearValueCount = 2;
			renderPassInfo.renderArea.extent.width = (uint32_t)size.width;
			renderPassInfo.renderArea.extent.height = (uint32_t)size.height;

			VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
			{
				VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->vkPipeline()));

				VkViewport viewport{};
				viewport.x = 0;
				viewport.y = 0;
				viewport.width = (float)size.width;
				viewport.height = (float)size.height;
				viewport.minDepth = 0;
				viewport.maxDepth = 1;

				VkRect2D scissor{};
				scissor.offset = {0, 0};
				scissor.extent = {(uint32_t)size.width, (uint32_t)size.height};

				VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
				VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));

				Matrix4 view = milo::lookAt(Vector3(0.0f, 0.0f, 3.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
				Matrix4 proj = milo::perspective(radians(45.0f), (float)size.width / (float)size.height, 0.1f, 100.0f);

				Matrix4 projView = proj * view;

				Matrix4 modelMatrix(1.0f);

				Matrix4 pushConstants[] = {projView, modelMatrix};

				VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(),
											VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Matrix4) * 2, pushConstants));

				const auto& materialResources = dynamic_cast<const VulkanMaterialResourcePool&>(Assets::materials().resourcePool());

				uint32_t dynamicOffset;

				VkDescriptorSet descriptorSet = materialResources.descriptorSetOf(material, dynamicOffset);

				VK_CALLV(vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->pipelineLayout(),
												 0, 1, &descriptorSet, 1, &dynamicOffset));

				const auto* buffers = dynamic_cast<const VulkanMeshBuffers*>(mesh->buffers());

				VkBuffer buffer[] = {buffers->vertexBuffer()->vkBuffer()};
				VkDeviceSize offset[] = {0};
				VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffer, offset));

				if(!mesh->indices().empty()) {
					VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, buffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
				}

				if(mesh->indices().empty()) {
					VK_CALLV(vkCmdDraw(commandBuffer, mesh->vertices().size(), 1, 0, 0));
				} else {
					VK_CALLV(vkCmdDrawIndexed(commandBuffer, mesh->indices().size(), 1, 0, 0, 0));
				}

			}
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		});

		VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), framebuffer, nullptr));

		iconTexture->setLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		return iconTexture;
	}

	void VulkanIconFactory::createRenderPass() {

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createColorAttachment(VK_FORMAT_R8G8B8A8_UNORM);

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

	void VulkanIconFactory::createDepthTexture(const Size& size) {

		m_DepthTexture = VulkanTexture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

		Texture2D::AllocInfo allocInfo = {};
		allocInfo.width = size.width;
		allocInfo.height = size.height;
		allocInfo.mipLevels = 1;
		allocInfo.format = PixelFormat::DEPTH;

		m_DepthTexture->allocate(allocInfo);
	}

	void VulkanIconFactory::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo createInfo{};

		createInfo.vkRenderPass = m_RenderPass;
		createInfo.setLayouts.push_back(dynamic_cast<VulkanMaterialResourcePool&>(Assets::materials().resourcePool()).materialDescriptorSetLayout());
		createInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		createInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPushConstantRange pushConstants{};
		pushConstants.offset = 0;
		pushConstants.size = sizeof(Matrix4) * 2;
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		createInfo.pushConstantRanges.push_back(pushConstants);

		createInfo.shaders.push_back({"resources/shaders/icons/bake_icon.vert", VK_SHADER_STAGE_VERTEX_BIT});
		createInfo.shaders.push_back({"resources/shaders/icons/bake_icon.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanIconFactoryGraphicsPipeline", m_Device, createInfo);
	}

	VulkanTexture2D *VulkanIconFactory::createTexture2D(const Size& size) {

		VulkanTexture2D* texture = VulkanTexture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);

		Texture2D::AllocInfo allocInfo{};
		allocInfo.width = size.width;
		allocInfo.height = size.height;
		allocInfo.mipLevels = 1;
		allocInfo.format = PixelFormat::RGBA8;

		texture->allocate(allocInfo);

		return texture;
	}

	VkFramebuffer VulkanIconFactory::createFramebuffer(VulkanTexture2D* colorAttachment, const Size& size) {

		VkImageView attachments[] = {colorAttachment->vkImageView(), m_DepthTexture->vkImageView()};

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = m_RenderPass;
		createInfo.width = size.width;
		createInfo.height = size.height;
		createInfo.layers = 1;
		createInfo.pAttachments = attachments;
		createInfo.attachmentCount = 2;

		VkFramebuffer framebuffer;
		VK_CALL(vkCreateFramebuffer(m_Device->logical(), &createInfo, nullptr, &framebuffer));

		return framebuffer;
	}

}

#include "milo/graphics/vulkan/rendering/passes/VulkanGeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"

namespace milo {

	VulkanGeometryRenderPass::VulkanGeometryRenderPass() {
		m_Device = VulkanContext::get()->device();
	}

	VulkanGeometryRenderPass::~VulkanGeometryRenderPass() {
		// TODO
	}

	void VulkanGeometryRenderPass::compile(FrameGraphResourcePool* resourcePool) {

		if(m_RenderPass == VK_NULL_HANDLE) {
			createRenderPass();
		}

		createFramebuffers(resourcePool);

		if(m_CameraUniformBuffer == nullptr) {
			createCameraUniformBuffer();
			createCameraDescriptorPool();
			createCameraDescriptorSets();
		}

		if(m_MaterialUniformBuffer == nullptr) {
			createMaterialUniformBuffer();
			createMaterialDescriptorPool();
			createMaterialDescriptorSets();
		}

		if(m_GraphicsPipeline == VK_NULL_HANDLE) {
			createPipelineLayout();
			createGraphicsPipeline();
		}

		if(m_CommandPool == nullptr) {
			createCommandPool();
			createCommandBuffers();
		}
	}

	void VulkanGeometryRenderPass::execute(Scene* scene) {
		// TODO
	}

	void VulkanGeometryRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = m_Device->depthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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

		Size size = Window::get()->size();

		VkFramebufferCreateInfo createInfo = mvk::FramebufferCreateInfo::create(m_RenderPass, size.width, size.height);

		VulkanFrameGraphResourcePool* pool = dynamic_cast<VulkanFrameGraphResourcePool*>(resourcePool);

		auto& colorTextures = pool->getTextures2D(m_Output.textures[0].handle);
		auto& depthTextures = pool->getTextures2D(m_Output.textures[1].handle);

		for(uint32_t i = 0;i < colorTextures.size();++i) {

			const VulkanTexture2D* colorTexture = dynamic_cast<const VulkanTexture2D*>(colorTextures[i].texture);
			const VulkanTexture2D* depthTexture = dynamic_cast<const VulkanTexture2D*>(depthTextures[i].texture);

			VkImageView attachments[2] = { colorTexture->vkImageView(), depthTexture->vkImageView() };

			createInfo.pAttachments = attachments;
			createInfo.attachmentCount = 2;

			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &createInfo, nullptr, &m_Framebuffers[i]));
		}

	}

	void VulkanGeometryRenderPass::createCameraUniformBuffer() {

	}

	void VulkanGeometryRenderPass::createCameraDescriptorPool() {

	}

	void VulkanGeometryRenderPass::createCameraDescriptorSets() {

	}

	void VulkanGeometryRenderPass::createMaterialUniformBuffer() {

	}

	void VulkanGeometryRenderPass::createMaterialDescriptorPool() {

	}

	void VulkanGeometryRenderPass::createMaterialDescriptorSets() {

	}

	void VulkanGeometryRenderPass::createPipelineLayout() {

	}

	void VulkanGeometryRenderPass::createGraphicsPipeline() {

	}

	void VulkanGeometryRenderPass::createCommandPool() {

	}

	void VulkanGeometryRenderPass::createCommandBuffers() {

	}
}
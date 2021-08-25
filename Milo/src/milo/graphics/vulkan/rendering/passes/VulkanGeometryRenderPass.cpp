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
			createCameraDescriptorLayout();
			createCameraDescriptorPool();
			createCameraDescriptorSets();
		}

		if(m_MaterialUniformBuffer == nullptr) {
			createMaterialUniformBuffer();
			createMaterialDescriptorLayout();
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

		VkAttachmentDescription colorAttachment = mvk::AttachmentDescription::createColorAttachment(VK_FORMAT_R32G32B32A32_SFLOAT);

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
		m_CameraUniformBuffer = new VulkanUniformBuffer<CameraData>();
		m_CameraUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanGeometryRenderPass::createCameraDescriptorLayout() {

		Array<VkDescriptorSetLayoutBinding, 1> bindings = {};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();

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

	void VulkanGeometryRenderPass::createMaterialUniformBuffer() {
		m_MaterialUniformBuffer = new VulkanUniformBuffer<MaterialData>();
		m_MaterialUniformBuffer->allocate(MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanGeometryRenderPass::createMaterialDescriptorLayout() {

		Array<VkDescriptorSetLayoutBinding, 1> bindings = {};
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_MaterialDescriptorSetLayout));
	}

	void VulkanGeometryRenderPass::createMaterialDescriptorPool() {

		VulkanDescriptorPool::CreateInfo createInfo = {};
		createInfo.capacity = MAX_SWAPCHAIN_IMAGE_COUNT;
		createInfo.layout = m_MaterialDescriptorSetLayout;

		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSize.descriptorCount = MAX_SWAPCHAIN_IMAGE_COUNT;

		createInfo.poolSizes.push_back(poolSize);

		m_MaterialDescriptorPool = new VulkanDescriptorPool(m_Device, createInfo);
	}

	void VulkanGeometryRenderPass::createMaterialDescriptorSets() {

		m_MaterialDescriptorPool->allocate(MAX_SWAPCHAIN_IMAGE_COUNT, [&](uint32_t index, VkDescriptorSet descriptorSet) {

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = m_MaterialUniformBuffer->vkBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(MaterialData);

			VkWriteDescriptorSet writeDescriptorSet = mvk::WriteDescriptorSet::createDynamicUniformBufferWrite(0, descriptorSet, 1, &bufferInfo);

			VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), 1, &writeDescriptorSet, 0, nullptr));
		});
	}

	void VulkanGeometryRenderPass::createPipelineLayout() {

		Array<VkDescriptorSetLayout, 2> setLayouts = {m_CameraDescriptorSetLayout, m_MaterialDescriptorSetLayout};

		VkPushConstantRange pushConstants = {};
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pSetLayouts = setLayouts.data();
		createInfo.setLayoutCount = setLayouts.size();
		createInfo.pPushConstantRanges = &pushConstants;
		createInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &createInfo, nullptr, &m_PipelineLayout));
	}

	void VulkanGeometryRenderPass::createGraphicsPipeline() {

	}

	void VulkanGeometryRenderPass::createCommandPool() {

	}

	void VulkanGeometryRenderPass::createCommandBuffers() {

	}
}
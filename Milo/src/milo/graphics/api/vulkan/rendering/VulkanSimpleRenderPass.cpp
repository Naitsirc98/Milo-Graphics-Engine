#include "milo/graphics/api/vulkan/rendering/VulkanSimpleRenderPass.h"
#include "milo/graphics/api/vulkan/VulkanDevice.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/api/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	static const ArrayList<float>& getCubeVertexData();

	VulkanSimpleRenderPass::VulkanSimpleRenderPass(VulkanSwapchain& swapchain) : m_Swapchain(swapchain) {

		create();

		m_Swapchain.addSwapchainRecreateCallback([&](){
			recreate();
		});
	}

	VulkanSimpleRenderPass::~VulkanSimpleRenderPass() {
		destroy();
	}

	void VulkanSimpleRenderPass::execute(uint32_t swapchainImageIndex) {

		Matrix4 view = lookAt(Vector3(0, 0, -3), Vector3(0, 0, 0), Vector3(0, 0, 1));
		Matrix4 proj = perspective(radians(45.0f), Window::get().aspectRatio(), 0.1f, 1000.0f);

		Matrix4 viewProj = proj * view;

		VkCommandBuffer commandBuffer = m_VkCommandBuffers[swapchainImageIndex];

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_VkRenderPass;
			renderPassInfo.framebuffer = m_VkFramebuffers[swapchainImageIndex];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = m_Swapchain.extent();

			VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline);

				VkBuffer vertexBuffers[] = {m_VertexBuffer->vkBuffer()};
				VkDeviceSize offsets[] = {0};
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

				for(uint32_t i = 0;i < 100;++i) {
					Matrix4 model = translate(Vector3(Random::nextInt(0, 100), Random::nextInt(0, 100), Random::nextInt(0, 100)));
					updatePushConstants(commandBuffer, viewProj * model);
					vkCmdDraw(commandBuffer, static_cast<uint32_t>(getCubeVertexData().size()), 1, 0, 0);
				}
			}
			vkCmdEndRenderPass(commandBuffer);
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));


	}

	void VulkanSimpleRenderPass::updatePushConstants(VkCommandBuffer commandBuffer, const Matrix4& mvp) {
		vkCmdPushConstants(commandBuffer, m_VkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp), value_ptr(mvp));
	}

	void VulkanSimpleRenderPass::create() {
		createRenderPass();
		createDepthTextures();
		createFramebuffers();
		createPipelineLayout();
		createGraphicsPipeline();
		createCommandPool();
		allocateCommandBuffers();
	}

	void VulkanSimpleRenderPass::destroy() {
		VkDevice device = m_Swapchain.device().ldevice();

		m_CommandPool->free(MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);
		DELETE_PTR(m_CommandPool);

		vkDestroyPipeline(device, m_VkGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, m_VkPipelineLayout, nullptr);

		for(auto & m_VkFramebuffer : m_VkFramebuffers) {
			vkDestroyFramebuffer(device, m_VkFramebuffer, nullptr);
		}

		vkDestroyRenderPass(device, m_VkRenderPass, nullptr);
	}

	void VulkanSimpleRenderPass::recreate() {
		destroy();
		create();
	}

	void VulkanSimpleRenderPass::createRenderPass() {

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_Swapchain.format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		colorAttachment.format = m_Swapchain.device().depthFormat();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.colorAttachmentCount = 1;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pDependencies = &dependency;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.subpassCount = 1;

		VK_CALL(vkCreateRenderPass(m_Swapchain.device().ldevice(), &renderPassInfo, nullptr, &m_VkRenderPass));
	}

	void VulkanSimpleRenderPass::createDepthTextures() {
		VkFormat format = m_Swapchain.device().depthFormat();

		Size size = Window::get().size();
		VkExtent3D extent = {(uint32_t)size.width, (uint32_t)size.height, 1};

		uint32_t queueFamilies[] = {m_Swapchain.device().graphicsQueue().family, m_Swapchain.device().presentationQueue().family};

		VulkanTextureAllocInfo allocInfo = {};
		allocInfo.imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		allocInfo.imageInfo.extent = extent;
		allocInfo.imageInfo.pQueueFamilyIndices = queueFamilies;
		allocInfo.imageInfo.queueFamilyIndexCount = queueFamilies[0] == queueFamilies[1] ? 1 : 2;
		allocInfo.viewInfo.format = format;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			auto* texture = NEW VulkanTexture(m_Swapchain.device());
			texture->allocate(allocInfo);
			m_DepthTextures[i] = texture;
		}
	}

	void VulkanSimpleRenderPass::createFramebuffers() {
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.renderPass = m_VkRenderPass;
		framebufferInfo.layers = 1;

		const VulkanSwapchainImage* swapchainImages = m_Swapchain.images();
		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VkImageView attachments[] = {swapchainImages[i].vkImageView, m_DepthTextures[i]->vkImageView()};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(m_Swapchain.device().ldevice(), &framebufferInfo, nullptr, &m_VkFramebuffers[i]));
		}
	}

	void VulkanSimpleRenderPass::createPipelineLayout() {
		VkPushConstantRange pushConstants = {};
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(Matrix4);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstants;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Swapchain.device().ldevice(), &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout));
	}

	void VulkanSimpleRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipelineInfo pipelineInfo;
		pipelineInfo.vkPipelineLayout = m_VkPipelineLayout;
		pipelineInfo.vkRenderPass = m_VkRenderPass;
		pipelineInfo.vkPipelineCache = m_VkPipelineCache;

		pipelineInfo.shaderInfos.push_back({"shaders/simple/simple.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"shaders/simple/simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_VkGraphicsPipeline = VulkanGraphicsPipeline::create(m_Swapchain.device().ldevice(), pipelineInfo);
	}

	void VulkanSimpleRenderPass::createCommandPool() {
		m_CommandPool = NEW VulkanCommandPool(m_Swapchain.device().graphicsQueue());
	}

	void VulkanSimpleRenderPass::allocateCommandBuffers() {
		m_CommandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);
	}

	void VulkanSimpleRenderPass::createVertexBuffer() {
		m_VertexBuffer = NEW VulkanBuffer(m_Swapchain.device());

		const ArrayList<float>& cubeVertexData = getCubeVertexData();

		VulkanBufferAllocInfo allocInfo = {};
		allocInfo.bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		allocInfo.bufferInfo.queueFamilyIndexCount = 1;
		allocInfo.bufferInfo.pQueueFamilyIndices = &m_Swapchain.device().graphicsQueue().family;
		allocInfo.bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		allocInfo.bufferInfo.size = cubeVertexData.size();
		allocInfo.data = cubeVertexData.data();

		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_VertexBuffer->allocate(allocInfo);
	}

	const ArrayList<float> CUBE_VERTEX_DATA = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
			1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f // bottom-left
	};

	static const ArrayList<float>& getCubeVertexData() {
		return CUBE_VERTEX_DATA;
	}
}
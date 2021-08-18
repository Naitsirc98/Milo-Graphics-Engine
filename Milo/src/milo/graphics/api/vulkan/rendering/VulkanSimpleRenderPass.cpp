#include <milo/scenes/components/Transform.h>
#include "milo/graphics/api/vulkan/rendering/VulkanSimpleRenderPass.h"
#include "milo/graphics/api/vulkan/VulkanContext.h"
#include "milo/graphics/api/vulkan/rendering/VulkanGraphicsPipeline.h"

namespace milo {

	static const ArrayList<float>& getCubeVertexData();

	VulkanSimpleRenderPass::VulkanSimpleRenderPass(VulkanSwapchain& swapchain) : m_Swapchain(swapchain) {
		create();
	}

	VulkanSimpleRenderPass::~VulkanSimpleRenderPass() {
		destroy();
	}

	void VulkanSimpleRenderPass::execute(const VulkanSimpleRenderPass::ExecuteInfo& executeInfo) {

		uint32_t swapchainImageIndex = executeInfo.swapchainImageIndex;

		Vector3 cameraPos   = Vector3(0.0f, 0.0f,  3.0f);
		Vector3 cameraFront = Vector3(0.0f, 0.0f, -1.0f);
		Vector3 cameraUp    = Vector3(0.0f, 1.0f,  0.0f);
		Matrix4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		Matrix4 proj = perspective(radians(45.0f), Window::get().aspectRatio(), 0.01f, 100.0f);

		Matrix4 viewProj = proj * view;

		VkCommandBuffer commandBuffer = m_VkCommandBuffers[swapchainImageIndex];

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		{
			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_VkRenderPass;
			renderPassInfo.framebuffer = m_VkFramebuffers[swapchainImageIndex];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = m_Swapchain.extent();

			VkClearValue clearValues[2];
			clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.clearValueCount = 2;
			renderPassInfo.pClearValues = clearValues;

			VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE));
			{
				VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline));

				VkBuffer vertexBuffers[] = {m_VertexBuffer->vkBuffer()};
				VkDeviceSize offsets[] = {0};
				VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

				for(int32_t x = -10;x <= 10;++x) {
					for(int32_t y = -10;y <= 10;++y) {

						float scale = 0.5f;

						Transform transform;
						transform.translation = {sin(Time::now()) + scale * 1.1f, y, -5};
						transform.scale = {scale, scale, scale};
						Matrix4 model = transform.modelMatrix();

						float r = cos((float) x * Time::now());
						float g = sin((float) y * Time::now());
						float b = r * g;

						Vector4 color = {r, g, b, 1.0f};

						PushConstants pushConstants = {};
						pushConstants.mvp = viewProj * model;
						pushConstants.color = color;

						updatePushConstants(commandBuffer, pushConstants);
						VK_CALLV(vkCmdDraw(commandBuffer, 36, 1, 0, 0));
					}
				}
			}
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = executeInfo.waitSemaphores;
		submitInfo.waitSemaphoreCount = executeInfo.waitSemaphoresCount;
		submitInfo.pSignalSemaphores = executeInfo.signalSemaphores;
		submitInfo.signalSemaphoreCount = executeInfo.signalSemaphoresCount;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitDstStageMask = executeInfo.waitDstStageMask;

		VK_CALL(vkQueueSubmit(m_Swapchain.device().graphicsQueue().vkQueue, 1, &submitInfo, executeInfo.fence));
	}

	void VulkanSimpleRenderPass::updatePushConstants(VkCommandBuffer commandBuffer, const PushConstants& pushConstants) {
		VK_CALLV(vkCmdPushConstants(commandBuffer,
									m_VkPipelineLayout,
									VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
									0,
									sizeof(pushConstants),
									&pushConstants));
	}

	void VulkanSimpleRenderPass::create() {
		createRenderPass();
		createRenderAreaDependentComponents();
		createPipelineLayout();
		createGraphicsPipeline();
		createCommandPool();
		allocateCommandBuffers();
		createVertexBuffer();
	}

	void VulkanSimpleRenderPass::destroy() {
		VkDevice device = m_Swapchain.device().ldevice();

		DELETE_PTR(m_VertexBuffer);

		destroyRenderAreaDependentComponents();

		m_CommandPool->free(MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);
		DELETE_PTR(m_CommandPool);

		VK_CALLV(vkDestroyPipeline(device, m_VkGraphicsPipeline, nullptr));
		VK_CALLV(vkDestroyPipelineLayout(device, m_VkPipelineLayout, nullptr));

		VK_CALLV(vkDestroyRenderPass(device, m_VkRenderPass, nullptr));
	}

	void VulkanSimpleRenderPass::createRenderAreaDependentComponents() {
		createDepthTextures();
		createFramebuffers();
	}

	void VulkanSimpleRenderPass::destroyRenderAreaDependentComponents() {
		for(auto& framebuffer : m_VkFramebuffers) {
			VK_CALLV(vkDestroyFramebuffer(m_Swapchain.device().ldevice(), framebuffer, nullptr));
		}

		for(auto& depthTexture : m_DepthTextures) {
			DELETE_PTR(depthTexture);
		}
	}

	void VulkanSimpleRenderPass::recreate() {
		destroyRenderAreaDependentComponents();
		createRenderAreaDependentComponents();
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
		depthAttachment.format = m_Swapchain.device().depthFormat();
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

		VK_CALL(vkCreateRenderPass(m_Swapchain.device().ldevice(), &renderPassInfo, nullptr, &m_VkRenderPass));
	}

	void VulkanSimpleRenderPass::createDepthTextures() {
		VkFormat format = m_Swapchain.device().depthFormat();

		Size size = Window::get().size();
		VkExtent3D extent = {(uint32_t)size.width, (uint32_t)size.height, 1};

		uint32_t queueFamilies[] = {m_Swapchain.device().graphicsQueue().family, m_Swapchain.device().presentationQueue().family};

		VulkanTextureAllocInfo allocInfo = {};
		allocInfo.imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		allocInfo.imageInfo.format = m_Swapchain.device().depthFormat();
		allocInfo.imageInfo.extent = extent;
		allocInfo.imageInfo.pQueueFamilyIndices = queueFamilies;
		allocInfo.imageInfo.queueFamilyIndexCount = queueFamilies[0] == queueFamilies[1] ? 1 : 2;
		allocInfo.viewInfo.format = format;
		allocInfo.viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			auto* texture = new VulkanTexture(m_Swapchain.device());
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
		framebufferInfo.width = m_Swapchain.extent().width;
		framebufferInfo.height = m_Swapchain.extent().height;

		const VulkanSwapchainImage* swapchainImages = m_Swapchain.images();
		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			VkImageView attachments[] = {swapchainImages[i].vkImageView, m_DepthTextures[i]->vkImageView()};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(m_Swapchain.device().ldevice(), &framebufferInfo, nullptr, &m_VkFramebuffers[i]));
		}
	}

	void VulkanSimpleRenderPass::createPipelineLayout() {
		VkPushConstantRange pushConstants = {};
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstants;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Swapchain.device().ldevice(), &pipelineLayoutInfo, nullptr,
									   &m_VkPipelineLayout));
	}

	void VulkanSimpleRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipelineInfo pipelineInfo;
		pipelineInfo.vkPipelineLayout = m_VkPipelineLayout;
		pipelineInfo.vkRenderPass = m_VkRenderPass;
		pipelineInfo.vkPipelineCache = m_VkPipelineCache;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/simple/simple.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/simple/simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_VkGraphicsPipeline = VulkanGraphicsPipeline::create("VulkanSimpleGraphicsPipeline", m_Swapchain.device().ldevice(), pipelineInfo);
	}

	void VulkanSimpleRenderPass::createCommandPool() {
		m_CommandPool = new VulkanCommandPool(m_Swapchain.device().graphicsQueue(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	}

	void VulkanSimpleRenderPass::allocateCommandBuffers() {
		m_CommandPool->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);
	}

	void VulkanSimpleRenderPass::createVertexBuffer() {
		VulkanDevice& device = m_Swapchain.device();
		m_VertexBuffer = new VulkanBuffer(device);

		uint32_t queueFamilies[] = {device.graphicsQueue().family, device.transferQueue().family};

		const ArrayList<float>& cubeVertexData = getCubeVertexData();
		const size_t vertexDataSize = cubeVertexData.size() * sizeof(float);

		VulkanBufferAllocInfo allocInfo = {};
		allocInfo.bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		allocInfo.bufferInfo.queueFamilyIndexCount = 1;
		allocInfo.bufferInfo.pQueueFamilyIndices = queueFamilies;
		allocInfo.bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		allocInfo.bufferInfo.size = vertexDataSize;
		allocInfo.dataInfo.data = cubeVertexData.data();
		allocInfo.dataInfo.commandPool = m_CommandPool;

		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		m_VertexBuffer->allocate(allocInfo);

		float vertices[288];
		m_VertexBuffer->readData(vertices, 288 * sizeof(float));

		Vector3 vertex0 = Vector3(vertices[0], vertices[1], vertices[2]);
	}

	static const ArrayList<float> CUBE_VERTEX_DATA = {
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
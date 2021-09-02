#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"
#include "milo/graphics/vulkan/VulkanContext.h"

#define IMGUI_IMPLEMENTATION
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace milo {

	VulkanUIRenderer::VulkanUIRenderer() {

		createRenderPass();
		createDepthBuffers();
		createFramebuffers();

		initUIBackend();
	}

	VulkanUIRenderer::~VulkanUIRenderer() {

		VulkanDevice* device = VulkanContext::get()->device();
		device->awaitTermination();

		for(VkFramebuffer framebuffer : m_Framebuffers) {
			VK_CALLV(vkDestroyFramebuffer(device->logical(), framebuffer, nullptr));
		}

		for(VulkanTexture2D* depthBuffer : m_DepthBuffers) {
			DELETE_PTR(depthBuffer);
		}

		VK_CALLV(vkDestroyRenderPass(device->logical(), m_RenderPass, nullptr));

		shutdownUIBackend();
	}

	void VulkanUIRenderer::shutdownUIBackend() const {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanUIRenderer::begin() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void VulkanUIRenderer::end() {

		ImGui::Render();

		VulkanDevice* device = VulkanContext::get()->device();
		VulkanSwapchain* swapchain = VulkanContext::get()->swapchain();
		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer primaryCommandBuffer = m_PrimaryCommandBuffers[imageIndex];
		VkCommandBuffer secondaryCommandBuffer = m_SecondaryCommandBuffers[imageIndex];

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f, 0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapchain->extent().width;
		uint32_t height = swapchain->extent().height;

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VK_CALL(vkBeginCommandBuffer(primaryCommandBuffer, &drawCmdBufInfo));

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = m_RenderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2; // Color + depth
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = m_Framebuffers[imageIndex];

		vkCmdBeginRenderPass(primaryCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = m_RenderPass;
		inheritanceInfo.framebuffer = m_Framebuffers[imageIndex];

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		VK_CALL(vkBeginCommandBuffer(secondaryCommandBuffer, &cmdBufInfo));

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(secondaryCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(secondaryCommandBuffer, 0, 1, &scissor);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), secondaryCommandBuffer);

		VK_CALL(vkEndCommandBuffer(secondaryCommandBuffer));

		vkCmdExecuteCommands(primaryCommandBuffer, 1, &secondaryCommandBuffer);

		vkCmdEndRenderPass(primaryCommandBuffer);

		VK_CALL(vkEndCommandBuffer(primaryCommandBuffer));

		VulkanQueue* queue = device->graphicsQueue();
		VulkanPresenter* presenter = VulkanContext::get()->vulkanPresenter();
		VkFence frameFence = presenter->frameInFlightFence();
		queue->waitSemaphores().push_back(presenter->imageAvailableSemaphore());
		VkSemaphore renderFinishedSemaphore = presenter->renderFinishedSemaphore();

		VkPipelineStageFlags waitStages[] = {
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers = &primaryCommandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pSignalSemaphores = &renderFinishedSemaphore;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pWaitDstStageMask = waitStages;

		queue->submit(submitInfo, frameFence);
		queue->clear();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanUIRenderer::initUIBackend() {

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoDecoration = false;
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.Fonts->AddFontFromFileTTF("resources/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("resources/fonts/opensans/OpenSans-Regular.ttf", 24.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//SetDarkThemeColors();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		Window* window = Window::get();

		VulkanContext* vulkanContext = VulkanContext::get();
		VulkanDevice* device = vulkanContext->device();
		VulkanSwapchain* swapchain = vulkanContext->swapchain();

		VkDescriptorPool descriptorPool;

		// Create Descriptor Pool
		ArrayList<VkDescriptorPoolSize> poolSizes = {
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolCreateInfo.maxSets = 100 * poolSizes.size();
		poolCreateInfo.poolSizeCount = poolSizes.size();
		poolCreateInfo.pPoolSizes = poolSizes.data();
		VK_CALL(vkCreateDescriptorPool(device->logical(), &poolCreateInfo, nullptr, &descriptorPool));

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window->handle(), true);

		ImGui_ImplVulkan_InitInfo imguiInitInfo = {};
		imguiInitInfo.Instance = vulkanContext->vkInstance();
		imguiInitInfo.PhysicalDevice = device->physical();
		imguiInitInfo.Device = device->logical();
		imguiInitInfo.QueueFamily = device->graphicsQueue()->family();
		imguiInitInfo.Queue = device->graphicsQueue()->vkQueue();
		imguiInitInfo.PipelineCache = nullptr;
		imguiInitInfo.DescriptorPool = descriptorPool;
		imguiInitInfo.Allocator = nullptr;
		imguiInitInfo.MinImageCount = 2;
		imguiInitInfo.ImageCount = swapchain->imageCount();
		imguiInitInfo.CheckVkResultFn = mvk::checkVkResult;
		ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);

		// Upload Fonts
		VulkanTask task;
		task.asynchronous = false;
		task.run = [&](VkCommandBuffer commandBuffer) {
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		};
		device->graphicsCommandPool()->execute(task);
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_PrimaryCommandBuffers.size(), m_PrimaryCommandBuffers.data());
		device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_SECONDARY, m_SecondaryCommandBuffers.size(), m_SecondaryCommandBuffers.data());
	}

	void VulkanUIRenderer::createRenderPass() {

		VulkanDevice* device = VulkanContext::get()->device();
		VulkanSwapchain* swapchain = VulkanContext::get()->swapchain();

		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapchain->format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = device->depthFormat();
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

		VK_CALL(vkCreateRenderPass(device->logical(), &renderPassInfo, nullptr, &m_RenderPass));
	}

	void VulkanUIRenderer::createDepthBuffers() {

		const Size size = Window::get()->size();

		PixelFormat depthFormat = mvk::toPixelFormat(VulkanContext::get()->device()->depthFormat());

		for(uint32_t i = 0;i < m_DepthBuffers.size();++i) {

			VulkanTexture2D* texture = VulkanTexture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			Texture2D::AllocInfo allocInfo = {};
			allocInfo.width = size.width;
			allocInfo.height = size.height;
			allocInfo.format = depthFormat;
			allocInfo.mipLevels = 1;

			texture->allocate(allocInfo);

			m_DepthBuffers[i] = texture;
		}
	}

	void VulkanUIRenderer::createFramebuffers() {

		VulkanSwapchain* swapchain = VulkanContext::get()->swapchain();
		VulkanDevice* device = VulkanContext::get()->device();

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.layers = 1;
		framebufferInfo.width = swapchain->extent().width;
		framebufferInfo.height = swapchain->extent().height;

		const VulkanSwapchainImage* swapchainImages = swapchain->images();
		for(uint32_t i = 0;i < m_Framebuffers.size();++i) {
			const VulkanSwapchainImage& colorTexture = swapchainImages[i];
			auto depthTexture = m_DepthBuffers[i];
			VkImageView attachments[] = {colorTexture.vkImageView, depthTexture->vkImageView()};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(device->logical(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}
}
#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/rendering/WorldRenderer.h"

#define IMGUI_IMPLEMENTATION
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace milo {

	VulkanUIRenderer::VulkanUIRenderer() {

		createRenderPass();
		createRenderAreaDependentResources();

		initUIBackend();

		Log::debug("Vulkan UI Renderer initialized");
	}

	void VulkanUIRenderer::createRenderAreaDependentResources() {
		createDepthBuffers();
		createFramebuffers();
	}

	VulkanUIRenderer::~VulkanUIRenderer() {

		destroy();
	}

	void VulkanUIRenderer::destroy() {

		destroyRenderAreaDependentResources();

		VK_CALLV(vkDestroyRenderPass(VulkanContext::get()->device()->logical(), m_RenderPass, nullptr));

		shutdownUIBackend();
	}

	VulkanDevice* VulkanUIRenderer::destroyRenderAreaDependentResources() {

		VulkanDevice* device = VulkanContext::get()->device();
		device->awaitTermination();

		for(VkFramebuffer framebuffer : m_Framebuffers) {
			VK_CALLV(vkDestroyFramebuffer(device->logical(), framebuffer, nullptr));
		}

		for(VulkanTexture2D* depthBuffer : m_DepthBuffers) {
			DELETE_PTR(depthBuffer);
		}
		return device;
	}

	void VulkanUIRenderer::shutdownUIBackend() const {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanUIRenderer::begin() {

		VulkanSwapchain* swapchain = VulkanContext::get()->swapchain();
		Size size = swapchain->size();

		if(m_FramebufferSize != size) {
			destroyRenderAreaDependentResources();
			createRenderAreaDependentResources();
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();

		auto texture = dynamic_cast<VulkanTexture2D*>(WorldRenderer::get().getFramebuffer().colorAttachments()[0]);
		texture->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanUIRenderer::end() {

		ImGui::Render();

		VulkanDevice* device = VulkanContext::get()->device();
		VulkanSwapchain* swapchain = VulkanContext::get()->swapchain();
		uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapchain->extent().width;
		uint32_t height = swapchain->extent().height;

		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;

		VK_CALL(vkBeginCommandBuffer(commandBuffer, &drawCmdBufInfo));
		{
			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = m_RenderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = width;
			renderPassBeginInfo.renderArea.extent.height = height;
			renderPassBeginInfo.clearValueCount = 1; // Color + depth
			renderPassBeginInfo.pClearValues = clearValues;
			renderPassBeginInfo.framebuffer = m_Framebuffers[imageIndex];

			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkCommandBufferInheritanceInfo inheritanceInfo = {};
			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.renderPass = m_RenderPass;
			inheritanceInfo.framebuffer = m_Framebuffers[imageIndex];

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor = {};
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent.width = width;
			scissor.extent.height = height;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
		}
		VK_CALL(vkEndCommandBuffer(commandBuffer));

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
		submitInfo.pCommandBuffers = &commandBuffer;
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
		setStyleColors();
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

		uint32_t numDescriptors = 256;

		// Create Descriptor Pool
		ArrayList<VkDescriptorPoolSize> poolSizes = {
				{ VK_DESCRIPTOR_TYPE_SAMPLER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, numDescriptors },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, numDescriptors }
		};

		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolCreateInfo.maxSets = numDescriptors * poolSizes.size();
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
		imguiInitInfo.PipelineCache = VK_NULL_HANDLE;
		imguiInitInfo.DescriptorPool = descriptorPool;
		imguiInitInfo.Allocator = VK_NULL_HANDLE;
		imguiInitInfo.MinImageCount = 2;
		imguiInitInfo.ImageCount = swapchain->imageCount();
		imguiInitInfo.CheckVkResultFn = mvk::checkVkResult;
		ImGui_ImplVulkan_Init(&imguiInitInfo, m_RenderPass);

		// Upload Fonts
		device->graphicsCommandPool()->execute([&](VkCommandBuffer commandBuffer) {
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		});
		device->awaitTermination();
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
		device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_SECONDARY, m_SecondaryCommandBuffers.size(), m_SecondaryCommandBuffers.data());
	}

	void VulkanUIRenderer::setStyleColors() {

		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

	}

	void VulkanUIRenderer::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::PresentationFormat, 1, RenderPass::LoadOp::Clear});

		m_RenderPass = mvk::RenderPass::create(desc);
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

		VulkanDevice* device = VulkanContext::get()->device();
		VulkanSwapchain* swapchain = device->context()->swapchain();

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.layers = 1;
		framebufferInfo.width = swapchain->extent().width;
		framebufferInfo.height = swapchain->extent().height;

		m_FramebufferSize = {(int32_t)swapchain->extent().width, (int32_t)swapchain->extent().height};

		const VulkanSwapchainImage* swapchainImages = swapchain->images();

		for(uint32_t i = 0;i < m_Framebuffers.size();++i) {
			const VulkanSwapchainImage& colorTexture = swapchainImages[i];
			VkImageView attachments[] = {colorTexture.vkImageView};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(device->logical(), &framebufferInfo, nullptr, &m_Framebuffers[i]));
		}
	}
}
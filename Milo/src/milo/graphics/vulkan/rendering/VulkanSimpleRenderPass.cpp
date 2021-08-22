#include <milo/scenes/components/Transform.h>
#include "milo/graphics/vulkan/rendering/VulkanSimpleRenderPass.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/rendering/VulkanGraphicsPipeline.h"
#include "milo/assets/images/Image.h"
#include "milo/io/Files.h"

namespace milo {

	static const ArrayList<float>& getCubeVertexData();

	VulkanSimpleRenderPass::VulkanSimpleRenderPass(VulkanSwapchain* swapchain)
		: m_Swapchain(swapchain), m_Device(swapchain->device()) {
		create();
	}

	VulkanSimpleRenderPass::~VulkanSimpleRenderPass() {
		destroy();
	}

	void VulkanSimpleRenderPass::execute(const VulkanSimpleRenderPass::Input& executeInfo) {

		uint32_t imageIndex=executeInfo.swapchainImageIndex;uint32_t swapchainImageIndex = imageIndex;

		Vector3 cameraPos   = Vector3(0.0f, 0.0f,  3.0f);
		Vector3 cameraFront = Vector3(0.0f, 0.0f, -1.0f);
		Vector3 cameraUp    = Vector3(0.0f, 1.0f,  0.0f);

		m_Camera.view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		m_Camera.proj = perspective(radians(45.0f), Window::get()->aspectRatio(), 0.01f, 100.0f);
		m_Camera.projView = m_Camera.proj * m_Camera.view;

		// This should be a per frame or per pass descriptor set
		//Camera cameraData = {view, proj, projView};
		//updateCameraUniformBuffer(executeInfo.swapchainImageIndex, cameraData);

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
			renderPassInfo.renderArea.extent = m_Swapchain->extent();

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

				for(int32_t obj = 0; obj < 50; ++obj) {

					uint32_t uniformIndex = obj + imageIndex * 64;

					float scale = sin(Time::now() * (obj + 1) * 0.1f);

					Transform transform;
					transform.translation = {sin(Time::now() * (obj + 1.0f)), obj, -4 - (obj+1) * 2};
					transform.scale = {scale, scale, scale};
					transform.rotate(Time::now(), Vector3(0, 1, 0));
					Matrix4 model = transform.modelMatrix();

					PushConstants pushConstants = {};
					pushConstants.mvp = model;
					updatePushConstants(commandBuffer, pushConstants);

					updateCameraUniformBuffer(uniformIndex, m_Camera);

					updateMaterialUniformBuffer(uniformIndex, m_Materials[obj % m_Materials.size()]);

					// offset per object!! (or batch of them) per imageIndex / frame
					//uint32_t dynamicOffset = executeInfo.swapchainImageIndex * obj * m_UniformBufferOffset * sizeof(Material);
					uint32_t dynamicOffset = uniformIndex * m_MaterialUniformBuffer->elementSize();

					VkDescriptorSet descriptorSet = m_DescriptorPool->get(uniformIndex);

					VK_CALLV(vkCmdBindDescriptorSets(commandBuffer,
													 VK_PIPELINE_BIND_POINT_GRAPHICS,
													 m_VkPipelineLayout,
													 0,
													 1,
													 &descriptorSet,
													 1,
													 &dynamicOffset));

					VK_CALLV(vkCmdDraw(commandBuffer, 36, 1, 0, 0));
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

		m_Device->graphicsQueue()->submit(1, &submitInfo, executeInfo.fence);
	}

	void VulkanSimpleRenderPass::updateCameraUniformBuffer(uint32_t swapchainImage, const Camera& camera) {
		m_CameraUniformBuffer->update(swapchainImage, camera);
	}

	void VulkanSimpleRenderPass::updatePushConstants(VkCommandBuffer commandBuffer, const PushConstants& pushConstants) {
		VK_CALLV(vkCmdPushConstants(commandBuffer,
									m_VkPipelineLayout,
									VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
									0,
									sizeof(pushConstants),
									&pushConstants));
	}

	void VulkanSimpleRenderPass::updateMaterialUniformBuffer(uint32_t swapchainImage, const Material& material) {
		m_MaterialUniformBuffer->update(swapchainImage, material);
	}

	void VulkanSimpleRenderPass::create() {
		createRenderPass();
		createRenderAreaDependentComponents();
		createCameraUniformBuffer();
		createMaterialUniformBuffer();
		createMaterials();
		createDescriptorSetLayout();
		createDescriptorPool();
		allocateDescriptorSets();
		createPipelineLayout();
		createGraphicsPipeline();
		createVertexBuffer();
		allocateCommandBuffers();
	}

	void VulkanSimpleRenderPass::destroy() {
		VkDevice device = m_Device->logical();

		for(auto & m_Material : m_Materials) {
			DELETE_PTR(m_Material.texture);
		}

		DELETE_PTR(m_Samplers);
		DELETE_PTR(m_VertexBuffer);

		DELETE_PTR(m_CameraUniformBuffer);
		DELETE_PTR(m_MaterialUniformBuffer);

		DELETE_PTR(m_DescriptorPool);
		VK_CALLV(vkDestroyDescriptorSetLayout(device, m_VkDescriptorSetLayout, nullptr));

		destroyRenderAreaDependentComponents();

		m_Device->graphicsCommandPool()->free(MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);

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
			VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), framebuffer, nullptr));
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
		colorAttachment.format = m_Swapchain->format();
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

		VK_CALL(vkCreateRenderPass(m_Device->logical(), &renderPassInfo, nullptr, &m_VkRenderPass));
	}

	void VulkanSimpleRenderPass::createDepthTextures() {

		const Size size = Window::get()->size();

		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {

			VulkanTexture2D* texture = VulkanTexture2D::createDepthAttachment();

			Texture2D::AllocInfo allocInfo = {};
			allocInfo.width = size.width;
			allocInfo.height = size.height;

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
		framebufferInfo.width = m_Swapchain->extent().width;
		framebufferInfo.height = m_Swapchain->extent().height;

		const VulkanSwapchainImage* swapchainImages = m_Swapchain->images();
		for(uint32_t i = 0;i < MAX_SWAPCHAIN_IMAGE_COUNT;++i) {
			const VulkanSwapchainImage& colorTexture = swapchainImages[i];
			auto depthTexture = m_DepthTextures[i];
			VkImageView attachments[] = {colorTexture.vkImageView, depthTexture->vkImageView()};
			framebufferInfo.pAttachments = attachments;
			VK_CALL(vkCreateFramebuffer(m_Device->logical(), &framebufferInfo, nullptr, &m_VkFramebuffers[i]));
		}
	}

	void VulkanSimpleRenderPass::createCameraUniformBuffer() {
		m_CameraUniformBuffer = VulkanUniformBuffer<Camera>::create();
		m_CameraUniformBuffer->allocate(64 * MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanSimpleRenderPass::createMaterialUniformBuffer() {
		m_MaterialUniformBuffer = VulkanUniformBuffer<Material>::create();
		m_MaterialUniformBuffer->allocate(64 * MAX_SWAPCHAIN_IMAGE_COUNT);
	}

	void VulkanSimpleRenderPass::createDescriptorSetLayout() {

		Array<VkDescriptorSetLayoutBinding, 3> bindings = {};
		// Camera Uniform Buffer
		bindings[0].binding = 0;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// Material Uniform Buffer
		bindings[1].binding = 1;
		bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		// Material TextureAsset
		bindings[2].binding = 2;
		bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bindings[2].descriptorCount = 1;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();

		VK_CALL(vkCreateDescriptorSetLayout(m_Device->logical(), &createInfo, nullptr, &m_VkDescriptorSetLayout));
	}

	void VulkanSimpleRenderPass::createDescriptorPool() {

		Array<VkDescriptorPoolSize, 3> poolSizes = {};
		// Camera Uniform Buffer
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = MAX_DESCRIPTOR_SETS; // Total count across all sets
		// Material Uniform Buffer
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = MAX_DESCRIPTOR_SETS; // Total count across all sets
		// Material TextureAsset
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = MAX_DESCRIPTOR_SETS; // Total count across all sets

		VulkanDescriptorPool::CreateInfo createInfo = {};
		createInfo.layout = m_VkDescriptorSetLayout;
		createInfo.capacity = MAX_DESCRIPTOR_SETS;
		createInfo.poolSizes.push_back(poolSizes[0]);
		createInfo.poolSizes.push_back(poolSizes[1]);
		createInfo.poolSizes.push_back(poolSizes[2]);

		m_DescriptorPool = new VulkanDescriptorPool(m_Swapchain->device(), createInfo);
	}

	void VulkanSimpleRenderPass::allocateDescriptorSets() {
		m_DescriptorPool->allocate(
				64 * MAX_SWAPCHAIN_IMAGE_COUNT,
				[&](size_t index, VkDescriptorSet vkDescriptorSet) { setupDescriptorSet(index, vkDescriptorSet);});
	}

	void VulkanSimpleRenderPass::setupDescriptorSet(size_t index, VkDescriptorSet vkDescriptorSet) {

		VkDescriptorBufferInfo cameraUBOInfo = {};
		cameraUBOInfo.offset = 0;
		cameraUBOInfo.range = sizeof(Camera);
		cameraUBOInfo.buffer = m_CameraUniformBuffer->vkBuffer();

		VkDescriptorBufferInfo materialUBOInfo = {};
		materialUBOInfo.offset = 0;
		materialUBOInfo.range = Material::size();
		materialUBOInfo.buffer = m_MaterialUniformBuffer->vkBuffer();

		VkDescriptorImageInfo materialTextureInfo = {};
		materialTextureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		size_t materialIndex = index % 64 % m_Materials.size();
		materialTextureInfo.imageView = m_Materials[materialIndex].texture->vkImageView();
		materialTextureInfo.sampler = m_Materials[materialIndex].texture->vkSampler();

		VkWriteDescriptorSet cameraUBOWrite = {};
		cameraUBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cameraUBOWrite.descriptorCount = 1;
		cameraUBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraUBOWrite.pBufferInfo = &cameraUBOInfo;
		cameraUBOWrite.dstSet = vkDescriptorSet;
		cameraUBOWrite.dstBinding = 0;

		VkWriteDescriptorSet materialUBOWrite = {};
		materialUBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialUBOWrite.descriptorCount = 1;
		materialUBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		materialUBOWrite.pBufferInfo = &materialUBOInfo;
		materialUBOWrite.dstSet = vkDescriptorSet;
		materialUBOWrite.dstBinding = 1;

		VkWriteDescriptorSet materialTextureWrite = {};
		materialTextureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialTextureWrite.descriptorCount = 1;
		materialTextureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialTextureWrite.pImageInfo = &materialTextureInfo;
		materialTextureWrite.dstSet = vkDescriptorSet;
		materialTextureWrite.dstBinding = 2;

		Array<VkWriteDescriptorSet, 3> writeDescriptorSets = {cameraUBOWrite, materialUBOWrite, materialTextureWrite};

		VK_CALLV(vkUpdateDescriptorSets(m_Device->logical(), writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr));
	}

	void VulkanSimpleRenderPass::createPipelineLayout() {
		VkPushConstantRange pushConstants = {};
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pSetLayouts = &m_VkDescriptorSetLayout;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstants;
		pipelineLayoutInfo.pushConstantRangeCount = 1;

		VK_CALL(vkCreatePipelineLayout(m_Device->logical(), &pipelineLayoutInfo, nullptr,
									   &m_VkPipelineLayout));
	}

	void VulkanSimpleRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipelineInfo pipelineInfo;
		pipelineInfo.vkPipelineLayout = m_VkPipelineLayout;
		pipelineInfo.vkRenderPass = m_VkRenderPass;
		pipelineInfo.vkPipelineCache = m_VkPipelineCache;

		pipelineInfo.depthStencil.depthTestEnable = VK_TRUE;

		pipelineInfo.shaderInfos.push_back({"resources/shaders/simple/simple.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaderInfos.push_back({"resources/shaders/simple/simple.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		m_VkGraphicsPipeline = VulkanGraphicsPipeline::create("VulkanSimpleGraphicsPipeline",
															  m_Device->logical(), pipelineInfo);
	}

	void VulkanSimpleRenderPass::allocateCommandBuffers() {
		m_Swapchain->device()->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, MAX_SWAPCHAIN_IMAGE_COUNT, m_VkCommandBuffers);
	}

	void VulkanSimpleRenderPass::createVertexBuffer() {

		m_VertexBuffer = VulkanBuffer::createVertexBuffer();

		const ArrayList<float>& vertices = getCubeVertexData();
		
		Buffer::AllocInfo allocInfo = {};
		allocInfo.size = vertices.size() * sizeof(float);
		allocInfo.data = vertices.data();

		m_VertexBuffer->allocate(allocInfo);
	}

	void VulkanSimpleRenderPass::createMaterials() {
		static ArrayList<String> imageFiles = Files::listFiles("C:/Users/naits/Pictures/Google Earth VR");

		createSamplersMap();

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 1.0f;
		samplerInfo.mipLodBias = 0;

		Array<Vector4, 5> colors = {
				Vector4(1.0f, 0.0f, 0.0f, 1.0f),
				Vector4(0.0f, 1.0f, 0.0f, 1.0f),
				Vector4(0.0f, 0.0f, 1.0f, 1.0f),
				Vector4(1.0f, 1.0f, 0.0f, 1.0f),
				Vector4(1.0f, 0.0f, 1.0f, 1.0f),
				};

		for(uint32_t i = 0;i < m_Materials.size();++i) {

			const String& imageFile = imageFiles[i % imageFiles.size()];
			Log::info("Loading texture {}...", imageFile);
			Image* image = Image::createImage(imageFile, PixelFormat::RGBA8);

			VulkanTexture2D::CreateInfo createInfo = {};
			createInfo.imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			createInfo.viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

			auto texture = new VulkanTexture2D(createInfo);
			texture->vkSampler(m_Samplers->get(samplerInfo));

			Texture2D::AllocInfo allocInfo = {};
			allocInfo.width = image->width();
			allocInfo.height = image->height();
			allocInfo.format = image->format();
			allocInfo.pixels = image->pixels();

			texture->allocate(allocInfo);
			texture->generateMipmaps();

			m_Materials[i].color = colors[i % colors.size()];
			m_Materials[i].texture = texture;

			DELETE_PTR(image);
		}
	}

	void VulkanSimpleRenderPass::createSamplersMap() {
		m_Samplers = new VulkanSamplerMap(m_Device);
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
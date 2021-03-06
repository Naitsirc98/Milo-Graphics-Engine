#include "milo/graphics/vulkan/VulkanAPI.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"
#include "milo/graphics/vulkan/descriptors/VulkanDescriptorPool.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	VulkanAPICall VulkanAPICallManager::s_LastVulkanAPICall;
	Mutex VulkanAPICallManager::s_Mutex;

	VulkanAPICall VulkanAPICallManager::popVkCall() {
		return s_LastVulkanAPICall;
	}

	void VulkanAPICallManager::pushVkCall(StackTrace&& stackTrace, const char* function, const char* file, uint32_t line) {
		s_Mutex.lock();
		{
			s_LastVulkanAPICall.stacktrace = std::forward<StackTrace>(stackTrace);
			s_LastVulkanAPICall.function = function;
			s_LastVulkanAPICall.file = file;
			s_LastVulkanAPICall.line = line;
		}
		s_Mutex.unlock();
	}

	VkResult VulkanAPICallManager::pushVkCall(StackTrace&& stackTrace, const char* function, const char* file,
											  uint32_t line, VkResult vkResult) {

		pushVkCall(std::forward<StackTrace>(stackTrace), function, file, line);
		return vkResult;
	}

	namespace mvk {

		void checkVkResult(VkResult vkResult) {
			if(vkResult != VK_SUCCESS) {
				Log::error("Vulkan call returned {}", getErrorName(vkResult));
			}
		}

		VkImageView getImageView(Texture2D* texture) {
			return ((VulkanTexture*)texture)->vkImageView();
		}

		VkImageView mvk::getImageView(Cubemap* cubemap) {
			return ((VulkanTexture*)cubemap)->vkImageView();
		}

		VkSampler getSampler(Texture2D* texture) {
			return ((VulkanTexture*)texture)->vkSampler();
		}

		VkSampler mvk::getSampler(Cubemap* cubemap) {
			return ((VulkanTexture*)cubemap)->vkSampler();
		}

		VkImageMemoryBarrier ImageMemoryBarrier::create(const VkImageViewCreateInfo& viewInfo,
														VkImageLayout oldLayout, VkImageLayout newLayout) noexcept {

			// GENERIC, MAY BE CHANGED BY CALLER
			VkImageSubresourceRange range = viewInfo.subresourceRange;

			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.subresourceRange = range;

			barrier.image = viewInfo.image;
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;

			// SHOULD BE SET BY CALLER IF OTHER BEHAVIOUR IS DESIRED
			if(oldLayout != newLayout) {
				if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
					barrier.srcAccessMask = 0;
					barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				} else {
					barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				}
			}

			return barrier;
		}


		VkImageCreateInfo ImageCreateInfo::create(TextureUsageFlags usage) noexcept {
			VkImageCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.arrayLayers = 1;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.mipLevels = 1;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

			if((usage & TEXTURE_USAGE_SAMPLED_BIT) != 0) imageInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
			if((usage & TEXTURE_USAGE_COLOR_ATTACHMENT_BIT) != 0) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			if((usage & TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			if((usage & TEXTURE_USAGE_STORAGE_BIT) != 0) imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

			return imageInfo;
		}

		VkImageCreateInfo ImageCreateInfo::colorAttachment() noexcept {
			VkImageCreateInfo imageInfo = create(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);
			imageInfo.format = VulkanContext::get()->swapchain()->format();
			return imageInfo;
		}

		VkImageCreateInfo ImageCreateInfo::depthStencilAttachment() noexcept {
			VkImageCreateInfo imageInfo = create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
			imageInfo.format = VulkanContext::get()->device()->depthFormat();
			return imageInfo;
		}


		// ====


		VkImageViewCreateInfo ImageViewCreateInfo::create(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
			VkImageViewCreateInfo viewInfo = {};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.image = image;
			viewInfo.format = format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = levelCount;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			return viewInfo;
		}

		VkImageViewCreateInfo ImageViewCreateInfo::colorAttachment(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
			return create(image, format, levelCount);
		}

		VkImageViewCreateInfo ImageViewCreateInfo::depthStencilAttachment(VkImage image, VkFormat format, uint32_t levelCount) noexcept {
			VkImageViewCreateInfo viewInfo = create(image, format, levelCount);
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			return viewInfo;
		}


		//  ====


		VkSamplerCreateInfo SamplerCreateInfo::create() noexcept {

			VulkanDevice* device = VulkanContext::get()->device();

			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = device->info().properties().limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0;
			samplerInfo.maxLod = 1.0f;
			samplerInfo.mipLodBias = 0;
			return samplerInfo;
		}
	}

	// =====

	VkBufferCreateInfo mvk::BufferCreateInfo::create(VkBufferUsageFlags usage) noexcept {
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.usage = usage;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.size = 0;
		createInfo.flags = 0;
		return createInfo;
	}

	// =====

	VkFramebufferCreateInfo mvk::FramebufferCreateInfo::create(VkRenderPass renderPass, uint32_t width, uint32_t height) noexcept {
		VkFramebufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.width = width;
		createInfo.height = height;
		createInfo.layers = 1;
		return createInfo;
	}

	// =====

	VkAttachmentDescription mvk::AttachmentDescription::createPresentSrcAttachment() {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = VulkanContext::get()->swapchain()->format();
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: get samples from context
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return colorAttachment;
	}

	VkAttachmentDescription mvk::AttachmentDescription::createColorAttachment(VkFormat format) {
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: get samples from context
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		return colorAttachment;
	}

	VkAttachmentDescription mvk::AttachmentDescription::createDepthStencilAttachment(VkFormat format) {

		if(format == VK_FORMAT_MAX_ENUM) {
			format = VulkanContext::get()->device()->depthFormat();
		}

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = format;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: get samples from context
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		return depthAttachment;
	}

	// =====

	namespace mvk::WriteDescriptorSet {

		VkWriteDescriptorSet create(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount) {
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstBinding = binding;
			writeDescriptorSet.descriptorCount = descriptorCount;
			writeDescriptorSet.dstSet = set;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createUniformBufferWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorBufferInfo* pBufferInfo) {
			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo = pBufferInfo;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createDynamicUniformBufferWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorBufferInfo* pBufferInfo) {
			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			writeDescriptorSet.pBufferInfo = pBufferInfo;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createStorageBufferWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorBufferInfo* pBufferInfo) {
			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			writeDescriptorSet.pBufferInfo = pBufferInfo;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createDynamicStorageBufferWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorBufferInfo* pBufferInfo) {
			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			writeDescriptorSet.pBufferInfo = pBufferInfo;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createCombineImageSamplerWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorImageInfo* pImageInfo) {
			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pImageInfo = pImageInfo;
			return writeDescriptorSet;
		}

		VkWriteDescriptorSet createStorageImageWrite(uint32_t binding, VkDescriptorSet set, uint32_t descriptorCount, VkDescriptorImageInfo* pImageInfo) {

			VkWriteDescriptorSet writeDescriptorSet = create(binding, set, descriptorCount);
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			writeDescriptorSet.pImageInfo = pImageInfo;
			return writeDescriptorSet;
		}

	}

	// =====

	namespace mvk::RenderPass {

		VkRenderPass create(const milo::RenderPass::Description& desc) {

			ArrayList<VkAttachmentDescription> attachments(desc.colorAttachments.size() + (desc.depthAttachment.has_value() ? 1 : 0));
			ArrayList<VkAttachmentReference> colorReferences;
			VkAttachmentReference depthStencilReference{};

			for(uint32_t i = 0;i < desc.colorAttachments.size();++i) {
				const milo::RenderPass::Attachment& colorAttachment = desc.colorAttachments[i];
				VkAttachmentDescription& attachment = attachments[i];
				attachment.format = mvk::fromPixelFormat(colorAttachment.format);
				attachment.samples = mvk::fromSamples(colorAttachment.samples);
				attachment.loadOp = mvk::fromLoadOp(colorAttachment.loadOp);
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachment.initialLayout = colorLayoutFromLoadOp(colorAttachment.loadOp);
				attachment.finalLayout = colorAttachment.format == PixelFormat::PresentationFormat
										 ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
										 : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				colorReferences.push_back({i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
			}

			if(desc.depthAttachment.has_value()) {
				const milo::RenderPass::Attachment& depthAttachment = desc.depthAttachment.value();
				VkAttachmentDescription& attachment = attachments[attachments.size() - 1];
				attachment.format = mvk::fromPixelFormat(depthAttachment.format);
				attachment.samples = mvk::fromSamples(depthAttachment.samples);
				attachment.loadOp = mvk::fromLoadOp(depthAttachment.loadOp);
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.stencilLoadOp = mvk::fromLoadOp(depthAttachment.loadOp);
				attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachment.initialLayout = depthLayoutFromLoadOp(depthAttachment.loadOp);
				attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				depthStencilReference.layout = attachment.finalLayout;
				depthStencilReference.attachment = attachments.size() - 1;
			}

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = colorReferences.size();
			if(desc.depthAttachment.has_value())
				subpass.pDepthStencilAttachment = &depthStencilReference;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.attachmentCount = attachments.size();
			renderPassInfo.pDependencies = &dependency;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.subpassCount = 1;

			VkRenderPass renderPass;
			VK_CALL(vkCreateRenderPass(VulkanContext::get()->device()->logical(), &renderPassInfo, nullptr, &renderPass));

			return renderPass;
		}
	}

	VkImageLayout mvk::RenderPass::colorLayoutFromLoadOp(milo::RenderPass::LoadOp loadOp) {
		switch(loadOp) {
			case milo::RenderPass::LoadOp::Load:
				return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			default:
				return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VkImageLayout mvk::RenderPass::depthLayoutFromLoadOp(milo::RenderPass::LoadOp loadOp) {
		switch(loadOp) {
			case milo::RenderPass::LoadOp::Load:
				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			default:
				return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	// =====

	namespace mvk::Semaphore {

		void create(uint32_t count, VkSemaphore* semaphores) {

			VkSemaphoreCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkDevice device = VulkanContext::get()->device()->logical();
			for(uint32_t i = 0;i < count;++i) {
				VK_CALL(vkCreateSemaphore(device, &createInfo, nullptr, &semaphores[i]));
			}
		}

		void destroy(uint32_t count, VkSemaphore* semaphores) {
			VkDevice device = VulkanContext::get()->device()->logical();
			for(uint32_t i = 0;i < count;++i) {
				VK_CALLV(vkDestroySemaphore(device, semaphores[i], nullptr));
			}
		}
	}

	// =====

	namespace mvk::CommandBuffer {

		inline static VulkanFramebuffer* prepareFramebuffer(VkCommandBuffer commandBuffer, const Framebuffer* fb) {
			VulkanFramebuffer* vulkanFramebuffer = (VulkanFramebuffer*)fb;
			for(Texture2D* colorAttachment : vulkanFramebuffer->colorAttachments()) {
				auto* texture = (VulkanTexture2D*)colorAttachment;
				if(texture->layout() != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
					texture->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				}
			}
			for(Texture2D* depthAttachment : vulkanFramebuffer->depthAttachments()) {
				auto* texture = (VulkanTexture2D*)depthAttachment;
				if(texture->layout() != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
					texture->setLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				}
			}
			return vulkanFramebuffer;
		}

		void beginGraphicsRenderPass(VkCommandBuffer commandBuffer, const BeginGraphicsRenderPassInfo& info) {

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			VK_CALL(vkBeginCommandBuffer(commandBuffer, &beginInfo));

			VkFramebuffer vkFramebuffer = VK_NULL_HANDLE;

			if(info.vkFramebuffer != VK_NULL_HANDLE) {
				vkFramebuffer = info.vkFramebuffer;
			} else {
				const Framebuffer* fb = info.framebuffer;
				if(fb == nullptr) {
					fb = &WorldRenderer::get().getFramebuffer();
				}
				VulkanFramebuffer* vulkanFramebuffer = prepareFramebuffer(commandBuffer, fb);
				vkFramebuffer = vulkanFramebuffer->get(info.renderPass);
			}

			Viewport theViewport{};

			if(info.viewport == nullptr) {
				theViewport = SceneManager::activeScene()->viewport();
			} else {
				theViewport = *info.viewport;
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = info.renderPass;
			renderPassInfo.framebuffer = vkFramebuffer;
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent.width = (uint32_t)theViewport.width;
			renderPassInfo.renderArea.extent.height = (uint32_t)theViewport.height;

			if(info.clearValues == nullptr) {
				throw MILO_RUNTIME_EXCEPTION("VkClearValues is null!!");
			}

			renderPassInfo.pClearValues = info.clearValues;
			renderPassInfo.clearValueCount = info.clearValuesCount;

			VK_CALLV(vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, info.subpassContents));

			VK_CALLV(vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, info.graphicsPipeline));

			if(info.dynamicViewport) {
				VkViewport viewport{};
				viewport.x = (float)theViewport.x;
				viewport.y = (float)theViewport.y;
				viewport.width = (float)theViewport.width;
				viewport.height = (float)theViewport.height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;

				VK_CALLV(vkCmdSetViewport(commandBuffer, 0, 1, &viewport));
			}

			if(info.dynamicScissor) {
				VkRect2D scissor{};
				scissor.offset = {0, 0};
				scissor.extent = {(uint32_t)theViewport.width, (uint32_t)theViewport.height};

				VK_CALLV(vkCmdSetScissor(commandBuffer, 0, 1, &scissor));
			}
		}

		void endGraphicsRenderPass(VkCommandBuffer commandBuffer) {
			VK_CALLV(vkCmdEndRenderPass(commandBuffer));
			VK_CALLV(vkEndCommandBuffer(commandBuffer));
		}
	}

	namespace mvk::DescriptorSet {

		VkDescriptorSetLayout Layout::create(const CreateInfo& createInfo) {

			VkDevice device = VulkanContext::get()->device()->logical();

			ArrayList<VkDescriptorSetLayoutBinding> bindings(createInfo.descriptors.size());

			for(uint32_t i = 0;i < bindings.size();++i) {
				VkDescriptorType descriptor = createInfo.descriptors[i];
				auto& binding = bindings[i];
				binding.binding = i;
				binding.descriptorType = descriptor;
				binding.descriptorCount = 1;
				binding.stageFlags = createInfo.stageFlags;
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.pBindings = bindings.data();
			layoutInfo.bindingCount = bindings.size();

			VkDescriptorSetLayout layout;
			VK_CALL(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout));
			return layout;
		}

		VulkanDescriptorPool* Pool::create(VkDescriptorSetLayout layout, const CreateInfo& createInfo) {

			VulkanDescriptorPool::CreateInfo poolInfo{};
			poolInfo.layout = layout;
			poolInfo.capacity = createInfo.numSets;

			for(auto descriptor : createInfo.descriptors) {
				poolInfo.poolSizes.push_back({descriptor, createInfo.numSets});
			}

			return new VulkanDescriptorPool(VulkanContext::get()->device(), poolInfo);
		}
	}

	// =====

	String mvk::getErrorName(VkResult vkResult) noexcept {
		switch (vkResult) {
			case VK_SUCCESS: return "VK_SUCCESS";
			case VK_NOT_READY: return "VK_NOT_READY";
			case VK_TIMEOUT: return "VK_TIMEOUT";
			case VK_EVENT_SET: return "VK_EVENT_SET";
			case VK_EVENT_RESET: return "VK_EVENT_RESET";
			case VK_INCOMPLETE: return "VK_INCOMPLETE";
			case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
			case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
			case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
			case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
			case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
			case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
			case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
			case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
			case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
			case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
			case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
			case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
			case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
			case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
			case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
			case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
			case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
			case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
			case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
			case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
			case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
			case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
			case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
			case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
			case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
			case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
			case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
		}
		return "Unknown Error";
	}

	VkSampleCountFlagBits mvk::fromSamples(uint32_t samples) {
		switch(samples) {
			case 1:
				return VK_SAMPLE_COUNT_1_BIT;
			case 2:
				return VK_SAMPLE_COUNT_2_BIT;
			case 4:
				return VK_SAMPLE_COUNT_4_BIT;
			case 8:
				return VK_SAMPLE_COUNT_8_BIT;
			case 16:
				return VK_SAMPLE_COUNT_16_BIT;
			case 32:
				return VK_SAMPLE_COUNT_32_BIT;
			case 64:
				return VK_SAMPLE_COUNT_64_BIT;
		}
		throw MILO_RUNTIME_EXCEPTION(fmt::format("Unsupported number of samples {}", samples));
	}

	VkAttachmentLoadOp mvk::fromLoadOp(milo::RenderPass::LoadOp loadOp) {
		switch(loadOp) {
			case milo::RenderPass::LoadOp::Undefined:
				return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			case milo::RenderPass::LoadOp::Clear:
				return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case milo::RenderPass::LoadOp::Load:
				return VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported attachment load operation");
	}

	PixelFormat mvk::toPixelFormat(VkFormat format) {

		if(format == VulkanContext::get()->device()->depthFormat()) {
			return PixelFormat::DEPTH;
		}

		switch (format) {
			case VK_FORMAT_R8_UNORM:
				return PixelFormat::R8;
			case VK_FORMAT_R8_SINT:
				return PixelFormat::R8I;
			case VK_FORMAT_R8_UINT:
				return PixelFormat::R8UI;
			case VK_FORMAT_R16_UNORM:
				return PixelFormat::R16;
			case VK_FORMAT_R16_SINT:
				return PixelFormat::R16I;
			case VK_FORMAT_R16_UINT:
				return PixelFormat::R16UI;
			case VK_FORMAT_R32_SINT:
				return PixelFormat::R32I;
			case VK_FORMAT_R32_UINT:
				return PixelFormat::R32UI;
			case VK_FORMAT_R16_SFLOAT:
				return PixelFormat::R16F;
			case VK_FORMAT_R32_SFLOAT:
				return PixelFormat::R32F;
			case VK_FORMAT_R8_SNORM:
				return PixelFormat::R8_SNORM;
			case VK_FORMAT_R16_SNORM:
				return PixelFormat::R16_SNORM;
			case VK_FORMAT_R8G8_UNORM:
				return PixelFormat::RG8;
			case VK_FORMAT_R8G8_SINT:
				return PixelFormat::RG8I;
			case VK_FORMAT_R8G8_UINT:
				return PixelFormat::RG8UI;
			case VK_FORMAT_R8G8_SNORM:
				return PixelFormat::RG8_SNORM;
			case VK_FORMAT_R16G16_UNORM:
				return PixelFormat::RG16;
			case VK_FORMAT_R16G16_SINT:
				return PixelFormat::RG16I;
			case VK_FORMAT_R16G16_UINT:
				return PixelFormat::RG16UI;
			case VK_FORMAT_R16G16_SFLOAT:
				return PixelFormat::RG16F;
			case VK_FORMAT_R32G32_SINT:
				return PixelFormat::RG32I;
			case VK_FORMAT_R32G32_UINT:
				return PixelFormat::RG32UI;
			case VK_FORMAT_R32G32_SFLOAT:
				return PixelFormat::RG32F;
			case VK_FORMAT_R16G16_SNORM:
				return PixelFormat::RG16_SNORM;
			case VK_FORMAT_R8G8B8_UNORM:
				return PixelFormat::RGB8;
			case VK_FORMAT_R8G8B8_SINT:
				return PixelFormat::RGB8I;
			case VK_FORMAT_R8G8B8_UINT:
				return PixelFormat::RGB8UI;
			case VK_FORMAT_R8G8B8_SNORM:
				return PixelFormat::RGB8_SNORM;
			case VK_FORMAT_R16G16B16_UNORM:
				return PixelFormat::RGB16;
			case VK_FORMAT_R16G16B16_SINT:
				return PixelFormat::RGB16I;
			case VK_FORMAT_R16G16B16_UINT:
				return PixelFormat::RGB16UI;
			case VK_FORMAT_R16G16B16_SFLOAT:
				return PixelFormat::RGB16F;
			case VK_FORMAT_R16G16B16_SNORM:
				return PixelFormat::RGB16_SNORM;
			case VK_FORMAT_R32G32B32_SINT:
				return PixelFormat::RGB32I;
			case VK_FORMAT_R32G32B32_UINT:
				return PixelFormat::RGB32UI;
			case VK_FORMAT_R32G32B32_SFLOAT:
				return PixelFormat::RGB32F;
			case VK_FORMAT_R8G8B8A8_UNORM:
				return PixelFormat::RGBA8;
			case VK_FORMAT_R8G8B8A8_SINT:
				return PixelFormat::RGBA8I;
			case VK_FORMAT_R8G8B8A8_UINT:
				return PixelFormat::RGBA8UI;
			case VK_FORMAT_R8G8B8A8_SNORM:
				return PixelFormat::RGBA8_SNORM;
			case VK_FORMAT_R16G16B16A16_UNORM:
				return PixelFormat::RGBA16;
			case VK_FORMAT_R16G16B16A16_SINT:
				return PixelFormat::RGBA16I;
			case VK_FORMAT_R16G16B16A16_UINT:
				return PixelFormat::RGBA16UI;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return PixelFormat::RGBA16F;
			case VK_FORMAT_R16G16B16A16_SNORM:
				return PixelFormat::RGBA16_SNORM;
			case VK_FORMAT_R32G32B32A32_SINT:
				return PixelFormat::RGBA32I;
			case VK_FORMAT_R32G32B32A32_UINT:
				return PixelFormat::RGBA32UI;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return PixelFormat::RGBA32F;
			case VK_FORMAT_R8G8B8_SRGB:
				return PixelFormat::SRGB;
			case VK_FORMAT_R8G8B8A8_SRGB:
				return PixelFormat::SRGBA;
			case VK_FORMAT_D32_SFLOAT:
				return PixelFormat::DEPTH32;
			default:
				throw MILO_RUNTIME_EXCEPTION("Unsupported pixel format");
		}
	}

	VkFormat mvk::fromPixelFormat(PixelFormat pixelFormat) {
		switch (pixelFormat) {
			case PixelFormat::R8:
				return VK_FORMAT_R8_UNORM;
			case PixelFormat::R8I:
				return VK_FORMAT_R8_SINT;
			case PixelFormat::R8UI:
				return VK_FORMAT_R8_UINT;
			case PixelFormat::R16:
				return VK_FORMAT_R16_UNORM;
			case PixelFormat::R16I:
				return VK_FORMAT_R16_SINT;
			case PixelFormat::R16UI:
				return VK_FORMAT_R16_UINT;
			case PixelFormat::R32I:
				return VK_FORMAT_R32_SINT;
			case PixelFormat::R32UI:
				return VK_FORMAT_R32_UINT;
			case PixelFormat::R16F:
				return VK_FORMAT_R16_SFLOAT;
			case PixelFormat::R32F:
				return VK_FORMAT_R32_SFLOAT;
			case PixelFormat::R8_SNORM:
				return VK_FORMAT_R8_SNORM;
			case PixelFormat::R16_SNORM:
				return VK_FORMAT_R16_SNORM;
			case PixelFormat::RG8:
				return VK_FORMAT_R8G8_UNORM;
			case PixelFormat::RG8I:
				return VK_FORMAT_R8G8_SINT;
			case PixelFormat::RG8UI:
				return VK_FORMAT_R8G8_UINT;
			case PixelFormat::RG8_SNORM:
				return VK_FORMAT_R8G8_SNORM;
			case PixelFormat::RG16:
				return VK_FORMAT_R16G16_UNORM;
			case PixelFormat::RG16I:
				return VK_FORMAT_R16G16_SINT;
			case PixelFormat::RG16UI:
				return VK_FORMAT_R16G16_UINT;
			case PixelFormat::RG16F:
				return VK_FORMAT_R16G16_SFLOAT;
			case PixelFormat::RG32I:
				return VK_FORMAT_R32G32_SINT;
			case PixelFormat::RG32UI:
				return VK_FORMAT_R32G32_UINT;
			case PixelFormat::RG32F:
				return VK_FORMAT_R32G32_SFLOAT;
			case PixelFormat::RG16_SNORM:
				return VK_FORMAT_R16G16_SNORM;
			case PixelFormat::RGB8:
				return VK_FORMAT_R8G8B8_UNORM;
			case PixelFormat::RGB8I:
				return VK_FORMAT_R8G8B8_SINT;
			case PixelFormat::RGB8UI:
				return VK_FORMAT_R8G8B8_UINT;
			case PixelFormat::RGB8_SNORM:
				return VK_FORMAT_R8G8B8_SNORM;
			case PixelFormat::RGB16:
				return VK_FORMAT_R16G16B16_UNORM;
			case PixelFormat::RGB16I:
				return VK_FORMAT_R16G16B16_SINT;
			case PixelFormat::RGB16UI:
				return VK_FORMAT_R16G16B16_UINT;
			case PixelFormat::RGB16F:
				return VK_FORMAT_R16G16B16_SFLOAT;
			case PixelFormat::RGB16_SNORM:
				return VK_FORMAT_R16G16B16_SNORM;
			case PixelFormat::RGB32I:
				return VK_FORMAT_R32G32B32_SINT;
			case PixelFormat::RGB32UI:
				return VK_FORMAT_R32G32B32_UINT;
			case PixelFormat::RGB32F:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case PixelFormat::RGBA8:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case PixelFormat::RGBA8I:
				return VK_FORMAT_R8G8B8A8_SINT;
			case PixelFormat::RGBA8UI:
				return VK_FORMAT_R8G8B8A8_UINT;
			case PixelFormat::RGBA8_SNORM:
				return VK_FORMAT_R8G8B8A8_SNORM;
			case PixelFormat::RGBA16:
				return VK_FORMAT_R16G16B16A16_UNORM;
			case PixelFormat::RGBA16I:
				return VK_FORMAT_R16G16B16A16_SINT;
			case PixelFormat::RGBA16UI:
				return VK_FORMAT_R16G16B16A16_UINT;
			case PixelFormat::RGBA16F:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
			case PixelFormat::RGBA16_SNORM:
				return VK_FORMAT_R16G16B16A16_SNORM;
			case PixelFormat::RGBA32I:
				return VK_FORMAT_R32G32B32A32_SINT;
			case PixelFormat::RGBA32UI:
				return VK_FORMAT_R32G32B32A32_UINT;
			case PixelFormat::RGBA32F:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case PixelFormat::SRGB:
				return VK_FORMAT_R8G8B8_SRGB;
			case PixelFormat::SRGBA:
				return VK_FORMAT_R8G8B8A8_SRGB;
			case PixelFormat::DEPTH:
				return VulkanContext::get()->device()->depthFormat();
			case PixelFormat::DEPTH32:
				return VK_FORMAT_D32_SFLOAT;
			case PixelFormat::PresentationFormat:
				return VulkanContext::get()->swapchain()->format();
			default:
				throw MILO_RUNTIME_EXCEPTION("Unsupported pixel format");
		}
	}

	VkShaderStageFlags mvk::fromShaderType(Shader::Type type) {
		switch(type) {
			case Shader::Type::Vertex:
				return VK_SHADER_STAGE_VERTEX_BIT;
			case Shader::Type::Fragment:
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			case Shader::Type::Geometry:
				return VK_SHADER_STAGE_GEOMETRY_BIT;
			case Shader::Type::TessControl:
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			case Shader::Type::TessEvaluation:
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			case Shader::Type::Compute:
				return VK_SHADER_STAGE_COMPUTE_BIT;
			case Shader::Type::Undefined:
			default:
				throw MILO_RUNTIME_EXCEPTION("Unknown shader type");
		}
	}
}

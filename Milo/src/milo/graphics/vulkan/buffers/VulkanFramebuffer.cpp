#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	VulkanFramebuffer::VulkanFramebuffer(const Framebuffer::CreateInfo& createInfo) : Framebuffer(createInfo) {

		const ApiInfo* apiInfo = (ApiInfo*)createInfo.apiInfo;

		m_Device = apiInfo->device;
		m_RenderPass = apiInfo->renderPass;

		create();
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		destroy();
	}

	VulkanDevice* VulkanFramebuffer::device() const {
		return m_Device;
	}

	VkRenderPass VulkanFramebuffer::renderPass() const {
		return m_RenderPass;
	}

	VkFramebuffer VulkanFramebuffer::vkFramebuffer() const {
		return m_Framebuffer;
	}

	void VulkanFramebuffer::resize(const Size& size) {

		if(m_Size == size) return;

		destroy();

		m_Size = size;

		for(auto& colorAttachment : m_ColorAttachments) {
			auto texture = dynamic_cast<VulkanTexture*>(colorAttachment.get());
			texture->resize(size);
		}

		for(auto& depthAttachment : m_DepthAttachments) {
			auto texture = dynamic_cast<VulkanTexture*>(depthAttachment.get());
			texture->resize(size);
		}

		create();
	}

	void VulkanFramebuffer::destroy() {
		VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), m_Framebuffer, nullptr));
		m_Framebuffer = VK_NULL_HANDLE;
	}

	void VulkanFramebuffer::create() {

		ArrayList<VkImageView> attachments;
		attachments.reserve(colorAttachments().size() + depthAttachments().size());

		for(const auto& colorAttachment : colorAttachments()) {
			const auto texture = dynamic_cast<const VulkanTexture2D*>(colorAttachment.get());
			attachments.push_back(texture->vkImageView());
		}

		for(const auto& depthAttachment : depthAttachments()) {
			const auto texture = dynamic_cast<const VulkanTexture2D*>(depthAttachment.get());
			attachments.push_back(texture->vkImageView());
		}

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = m_RenderPass;
		framebufferCreateInfo.width = m_Size.width;
		framebufferCreateInfo.height = m_Size.height;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();

		VK_CALL(vkCreateFramebuffer(m_Device->logical(), &framebufferCreateInfo, nullptr, &m_Framebuffer));
	}

}
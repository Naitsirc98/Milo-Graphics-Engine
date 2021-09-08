#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	VulkanFramebuffer::VulkanFramebuffer(const Framebuffer::CreateInfo& createInfo) : Framebuffer(createInfo) {

		const ApiInfo* apiInfo = (ApiInfo*)createInfo.apiInfo;
		m_Device = apiInfo->device;
	}

	VulkanFramebuffer::~VulkanFramebuffer() {
		destroyAllFramebuffers();
	}

	VulkanDevice* VulkanFramebuffer::device() const {
		return m_Device;
	}

	bool VulkanFramebuffer::hasFramebuffer(VkRenderPass renderPass) const {
		return m_VkFramebuffers.find(renderPass) != m_VkFramebuffers.end();
	}

	VkFramebuffer VulkanFramebuffer::get(VkRenderPass renderPass) {

		if(hasFramebuffer(renderPass)) {
			return m_VkFramebuffers.at(renderPass);
		}

		VkFramebuffer framebuffer = createFramebuffer(renderPass);
		m_VkFramebuffers[renderPass] = framebuffer;

		return framebuffer;
	}

	void VulkanFramebuffer::resize(const Size& size) {

		if(m_Size == size) return;
		m_Size = size;

		m_Device->awaitTermination();

		for(auto& colorAttachment : m_ColorAttachments) {
			auto texture = dynamic_cast<VulkanTexture*>(colorAttachment);
			texture->resize(size);
		}

		for(auto& depthAttachment : m_DepthAttachments) {
			auto texture = dynamic_cast<VulkanTexture*>(depthAttachment);
			texture->resize(size);
		}

		for(auto& [renderPass, framebuffer] : m_VkFramebuffers) {
			destroyFramebuffer(framebuffer);
			framebuffer = createFramebuffer(renderPass);
		}
	}

	void VulkanFramebuffer::destroyAllFramebuffers() {
		for(auto [renderPass, framebuffer] : m_VkFramebuffers) {
			destroyFramebuffer(framebuffer);
		}
		m_VkFramebuffers.clear();
	}

	void VulkanFramebuffer::destroyFramebuffer(VkFramebuffer framebuffer) {
		VK_CALLV(vkDestroyFramebuffer(m_Device->logical(), framebuffer, nullptr));
	}

	VkFramebuffer VulkanFramebuffer::createFramebuffer(VkRenderPass renderPass) {

		ArrayList<VkImageView> attachments;
		attachments.reserve(colorAttachments().size() + depthAttachments().size());

		for(const auto& colorAttachment : colorAttachments()) {
			const auto texture = dynamic_cast<const VulkanTexture2D*>(colorAttachment);
			attachments.push_back(texture->vkImageView());
		}

		for(const auto& depthAttachment : depthAttachments()) {
			const auto texture = dynamic_cast<const VulkanTexture2D*>(depthAttachment);
			attachments.push_back(texture->vkImageView());
		}

		VkFramebufferCreateInfo framebufferCreateInfo{};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.width = m_Size.width;
		framebufferCreateInfo.height = m_Size.height;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();

		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VK_CALL(vkCreateFramebuffer(m_Device->logical(), &framebufferCreateInfo, nullptr, &framebuffer));

		return framebuffer;
	}

}
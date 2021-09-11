#include "milo/graphics/rendering/Framebuffer.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"
#include "milo/graphics/vulkan/VulkanContext.h"

namespace milo {

	Framebuffer::Framebuffer(const Framebuffer::CreateInfo& createInfo) {

		m_Size = createInfo.size;

		for(PixelFormat format : createInfo.colorAttachments) {

			auto* colorAttachment = Texture2D::create(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT);

			Texture2D::AllocInfo allocInfo{};
			allocInfo.format = format;
			allocInfo.width = m_Size.width;
			allocInfo.height = m_Size.height;
			allocInfo.mipLevels = 1;

			colorAttachment->allocate(allocInfo);

			m_ColorAttachments.push_back(colorAttachment);
		}

		for(PixelFormat format : createInfo.depthAttachments) {

			auto* depthAttachment = Texture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT);

			Texture2D::AllocInfo allocInfo{};
			allocInfo.format = format;
			allocInfo.width = m_Size.width;
			allocInfo.height = m_Size.height;
			allocInfo.mipLevels = 1;

			depthAttachment->allocate(allocInfo);

			m_DepthAttachments.push_back(depthAttachment);
		}
	}

	Framebuffer::~Framebuffer() {

		for(Texture2D* colorAttachment : m_ColorAttachments) {
			DELETE_PTR(colorAttachment);
		}

		for(Texture2D* depthAttachment : m_DepthAttachments) {
			DELETE_PTR(depthAttachment);
		}
	}

	const ArrayList<Texture2D*>& Framebuffer::colorAttachments() const {
		return m_ColorAttachments;
	}

	const ArrayList<Texture2D*>& Framebuffer::depthAttachments() const {
		return m_DepthAttachments;
	}

	const Size& Framebuffer::size() const {
		return m_Size;
	}

	const String& Framebuffer::name() const {
		return m_Name;
	}

	void Framebuffer::setName(const String& name) {
		m_Name = name;
		for(uint32_t i = 0;i < m_ColorAttachments.size();++i) {
			m_ColorAttachments[i]->setName(name + "_ColorAttachment_" + str(i));
		}
		for(uint32_t i = 0;i < m_DepthAttachments.size();++i) {
			m_DepthAttachments[i]->setName(name + "_ColorAttachment_" + str(i));
		}
	}

	Framebuffer* Framebuffer::create(const Framebuffer::CreateInfo& createInfo) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			CreateInfo vulkanCreateInfo = createInfo;
			VulkanFramebuffer::ApiInfo apiInfo{};
			apiInfo.device = VulkanContext::get()->device();
			vulkanCreateInfo.apiInfo = &apiInfo;
			return new VulkanFramebuffer(vulkanCreateInfo);
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
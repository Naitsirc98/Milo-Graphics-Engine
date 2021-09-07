#include "milo/graphics/rendering/Framebuffer.h"

namespace milo {

	Framebuffer::Framebuffer(const Framebuffer::CreateInfo& createInfo) {

		m_Size = createInfo.size;

		for(PixelFormat format : createInfo.colorAttachments) {

			auto* colorAttachment = Texture2D::create(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT);

			Texture2D::AllocInfo allocInfo{};
			allocInfo.format = format;
			allocInfo.width = m_Size.width;
			allocInfo.height = m_Size.height;

			colorAttachment->allocate(allocInfo);
		}

		for(PixelFormat format : createInfo.depthAttachments) {

			auto* depthAttachment = Texture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			Texture2D::AllocInfo allocInfo{};
			allocInfo.format = format;
			allocInfo.width = m_Size.width;
			allocInfo.height = m_Size.height;

			depthAttachment->allocate(allocInfo);
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
}
#pragma once

#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Framebuffer {
	public:
		struct CreateInfo {
			ArrayList<PixelFormat> colorAttachments;
			ArrayList<PixelFormat> depthAttachments;
			Size size{};
			const void* apiInfo{nullptr};
		};
	protected:
		ArrayList<Ref<Texture2D>> m_ColorAttachments;
		ArrayList<Ref<Texture2D>> m_DepthAttachments;
		Size m_Size{};
	protected:
		Framebuffer(const CreateInfo& createInfo);
		virtual ~Framebuffer() = default;
	public:
		const ArrayList<Ref<Texture2D>>& colorAttachments() const;
		const ArrayList<Ref<Texture2D>>& depthAttachments() const;
		const Size& size() const;
		virtual void resize(const Size& size) = 0;
	};

}
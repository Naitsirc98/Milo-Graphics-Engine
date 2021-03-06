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
		ArrayList<Texture2D*> m_ColorAttachments;
		ArrayList<Texture2D*> m_DepthAttachments;
		Size m_Size{};
		String m_Name;
	public:
		Framebuffer(const CreateInfo& createInfo);
		virtual ~Framebuffer();
	public:
		const ArrayList<Texture2D*>& colorAttachments() const;
		const ArrayList<Texture2D*>& depthAttachments() const;
		const Size& size() const;
		const String& name() const;
		void setName(const String& name);
		virtual void resize(const Size& size) = 0;
	public:
		static Framebuffer* create(const CreateInfo& createInfo);
	};

}
#pragma once

#include "milo/common/Common.h"
#include "milo/assets/images/PixelFormat.h"
#include "milo/graphics/buffer/Buffer.h"
#include "milo/graphics/textures/Texture.h"
#include "milo/scenes/Scene.h"
#include "milo/graphics/rendering/Framebuffer.h"

namespace milo {

	class FrameGraphResourcePool {
		friend class WorldRenderer;
	protected:
		ArrayList<Framebuffer*> m_DefaultFramebuffers;
		HashMap<Handle, Ref<Framebuffer>> m_Framebuffers;
		HashMap<Handle, Ref<Buffer>> m_Buffers;
		HashMap<Handle, Ref<Texture2D>> m_Textures;
		HashMap<Handle, Ref<Cubemap>> m_Cubemaps;
	protected:
		FrameGraphResourcePool();
		virtual ~FrameGraphResourcePool();
		virtual void init();
	public:
		virtual void compile(Scene* scene);
		Framebuffer* getDefaultFramebuffer(uint32_t index = UINT32_MAX) const;
		Ref<Framebuffer> getFramebuffer(Handle handle) const;
		void putFramebuffer(Handle handle, Ref<Framebuffer> framebuffer);
		void removeFramebuffer(Handle handle);
		Ref<Buffer> getBuffer(Handle handle) const;
		void putBuffer(Handle handle, Ref<Buffer> buffer);
		void removeBuffer(Handle handle);
		Ref<Texture2D> getTexture2D(Handle handle) const;
		void putTexture2D(Handle handle, Ref<Texture2D> texture);
		void removeTexture2D(Handle handle);
		Ref<Cubemap> getCubemap(Handle handle) const;
		void putCubemap(Handle handle, Ref<Cubemap> cubemap);
		void removeCubemap(Handle handle);
	protected:
		virtual uint32_t currentFramebufferIndex() const = 0;
		virtual uint32_t maxDefaultFramebuffersCount() const = 0;
	private:
		void createDefaultFramebuffers(const Size& size);
	public:
		static FrameGraphResourcePool* create();
	};

}
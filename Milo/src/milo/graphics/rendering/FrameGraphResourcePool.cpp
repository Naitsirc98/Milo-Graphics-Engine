#include <milo/graphics/rendering/descriptions/ResourceDescriptions.h>
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"

namespace milo {

	FrameGraphResourcePool::FrameGraphResourcePool() {
	}

	FrameGraphResourcePool::~FrameGraphResourcePool() {
		for(Framebuffer* framebuffer : m_DefaultFramebuffers) {
			DELETE_PTR(framebuffer);
		}
		m_DefaultFramebuffers.clear();
	}

	void FrameGraphResourcePool::compile(Scene* scene) {

		const Size& sceneSize = scene->viewportSize();

		if(sceneSize != m_DefaultFramebuffers[0]->size()) {
			for(Framebuffer*& framebuffer : m_DefaultFramebuffers) {
				framebuffer->resize(sceneSize);
			}
		}
	}

	const Framebuffer* FrameGraphResourcePool::getDefaultFramebuffer(uint32_t index) const {
		return m_DefaultFramebuffers[index == UINT32_MAX ? currentFramebufferIndex() : index];
	}

	Ref<Framebuffer> FrameGraphResourcePool::getFramebuffer(Handle handle) const {
		return m_Framebuffers.find(handle) != m_Framebuffers.end() ? m_Framebuffers.at(handle) : nullptr;
	}

	void FrameGraphResourcePool::putFramebuffer(Handle handle, Ref<Framebuffer> framebuffer) {
		m_Framebuffers[handle] = framebuffer;
	}

	void FrameGraphResourcePool::removeFramebuffer(Handle handle) {
		m_Framebuffers.erase(handle);
	}

	Ref<Buffer> FrameGraphResourcePool::getBuffer(Handle handle) const {
		return m_Buffers.find(handle) != m_Buffers.end() ? m_Buffers.at(handle) : nullptr;
	}

	void FrameGraphResourcePool::putBuffer(Handle handle, Ref<Buffer> buffer) {
		m_Buffers[handle] = buffer;
	}

	void FrameGraphResourcePool::removeBuffer(Handle handle) {
		m_Buffers.erase(handle);
	}

	Ref<Texture2D> FrameGraphResourcePool::getTexture2D(Handle handle) const {
		return m_Textures.find(handle) != m_Textures.end() ? m_Textures.at(handle) : nullptr;
	}

	void FrameGraphResourcePool::putTexture2D(Handle handle, Ref<Texture2D> texture) {
		m_Textures[handle] = texture;
	}

	void FrameGraphResourcePool::removeTexture2D(Handle handle) {
		m_Textures.erase(handle);
	}

	Ref<Cubemap> FrameGraphResourcePool::getCubemap(Handle handle) const {
		return m_Cubemaps.find(handle) != m_Cubemaps.end() ? m_Cubemaps.at(handle) : nullptr;
	}

	void FrameGraphResourcePool::putCubemap(Handle handle, Ref<Cubemap> cubemap) {
		m_Cubemaps[handle] = cubemap;
	}

	void FrameGraphResourcePool::removeCubemap(Handle handle) {
		m_Cubemaps.erase(handle);
	}

	FrameGraphResourcePool* FrameGraphResourcePool::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFrameGraphResourcePool();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}


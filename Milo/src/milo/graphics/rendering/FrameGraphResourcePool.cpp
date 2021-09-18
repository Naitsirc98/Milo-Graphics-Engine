#include <milo/graphics/rendering/descriptions/ResourceDescriptions.h>
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	FrameGraphResourcePool::FrameGraphResourcePool() {
	}

	FrameGraphResourcePool::~FrameGraphResourcePool() {
		for(Framebuffer* framebuffer : m_DefaultFramebuffers) {
			DELETE_PTR(framebuffer);
		}
		m_DefaultFramebuffers.clear();
	}

	void FrameGraphResourcePool::init() {
		const Size& sceneSize = SceneManager::activeScene()->viewportSize();
		createDefaultFramebuffers(sceneSize);
	}

	void FrameGraphResourcePool::compile(Scene* scene) {

		const Size& sceneSize = scene->viewportSize();

		if(sceneSize != m_DefaultFramebuffers[0]->size()) {
			for(Framebuffer*& framebuffer : m_DefaultFramebuffers) {
				framebuffer->resize(sceneSize);
			}
		}
	}

	Framebuffer* FrameGraphResourcePool::getDefaultFramebuffer(uint32_t index) const {
		return m_DefaultFramebuffers[index == UINT32_MAX ? currentFramebufferIndex() : index];
	}

	Ref<Framebuffer> FrameGraphResourcePool::getFramebuffer(Handle handle) const {
		return m_Framebuffers.at(handle);
	}

	void FrameGraphResourcePool::putFramebuffer(Handle handle, Ref<Framebuffer> framebuffer) {
		m_Framebuffers[handle] = std::move(framebuffer);
	}

	void FrameGraphResourcePool::removeFramebuffer(Handle handle) {
		m_Framebuffers.erase(handle);
	}

	Ref<Buffer> FrameGraphResourcePool::getBuffer(Handle handle) const {
		return m_Buffers.at(handle);
	}

	void FrameGraphResourcePool::putBuffer(Handle handle, Ref<Buffer> buffer) {
		m_Buffers[handle] = std::move(buffer);
	}

	void FrameGraphResourcePool::removeBuffer(Handle handle) {
		m_Buffers.erase(handle);
	}

	Ref<Texture2D> FrameGraphResourcePool::getTexture2D(Handle handle) const {
		return m_Textures.at(handle);
	}

	void FrameGraphResourcePool::putTexture2D(Handle handle, Ref<Texture2D> texture) {
		m_Textures[handle] = std::move(texture);
	}

	void FrameGraphResourcePool::removeTexture2D(Handle handle) {
		m_Textures.erase(handle);
	}

	Ref<Cubemap> FrameGraphResourcePool::getCubemap(Handle handle) const {
		return m_Cubemaps.at(handle);
	}

	void FrameGraphResourcePool::putCubemap(Handle handle, Ref<Cubemap> cubemap) {
		m_Cubemaps[handle] = std::move(cubemap);
	}

	void FrameGraphResourcePool::removeCubemap(Handle handle) {
		m_Cubemaps.erase(handle);
	}

	void FrameGraphResourcePool::createDefaultFramebuffers(const Size& size) {

		uint32_t maxFramebufferCount = maxDefaultFramebuffersCount();

		m_DefaultFramebuffers.reserve(maxFramebufferCount);

		Framebuffer::CreateInfo createInfo{};
		createInfo.size = size;
		createInfo.colorAttachments.push_back(PixelFormat::RGBA32F);
		createInfo.depthAttachments.push_back(PixelFormat::DEPTH);

		for(uint32_t i = 0;i < maxFramebufferCount;++i) {
			Framebuffer* framebuffer = Framebuffer::create(createInfo);
			framebuffer->setName("DefaultFramebuffer[" + str(i) + "]");
			m_DefaultFramebuffers.push_back(framebuffer);
		}
	}

	FrameGraphResourcePool* FrameGraphResourcePool::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFrameGraphResourcePool();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}


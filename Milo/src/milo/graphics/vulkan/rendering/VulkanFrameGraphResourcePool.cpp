#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/presentation/VulkanPresenter.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	VulkanFrameGraphResourcePool::VulkanFrameGraphResourcePool() {
		m_Buffers.reserve(8);
		m_Textures.reserve(8);
	}

	VulkanFrameGraphResourcePool::~VulkanFrameGraphResourcePool() {

		VulkanContext::get()->device()->awaitTermination();

		for(auto& buffers : m_Buffers) {
			for(auto& buffer : buffers) {
				DELETE_PTR(buffer.buffer)
			}
		}

		for(auto& textures : m_Textures) {
			for(auto& texture : textures) {
				DELETE_PTR(texture.texture)
			}
		}

		for(RenderTarget& renderTarget : m_RenderTargets) {
			DELETE_PTR(renderTarget.colorAttachment);
			DELETE_PTR(renderTarget.depthAttachment);
		}
	}

	void VulkanFrameGraphResourcePool::update() {

		Scene* scene = SceneManager::activeScene();

		const Size& sceneSize = scene->viewportSize();

		if(sceneSize != m_RenderTargets[0].size) {

			for(RenderTarget& renderTarget : m_RenderTargets) {

				renderTarget.size = sceneSize;

				VulkanTexture2D* colorAttachment = dynamic_cast<VulkanTexture2D*>(renderTarget.colorAttachment);
				VulkanTexture2D* depthAttachment = dynamic_cast<VulkanTexture2D*>(renderTarget.depthAttachment);

				if(colorAttachment == nullptr) {
					colorAttachment = VulkanTexture2D::create(TEXTURE_USAGE_COLOR_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT);

					Texture2D::AllocInfo allocInfo{};
					allocInfo.width = sceneSize.width;
					allocInfo.height = sceneSize.height;
					allocInfo.format = PixelFormat::RGBA32F;
					allocInfo.mipLevels = 1;

					colorAttachment->allocate(allocInfo);

					colorAttachment->setLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

					renderTarget.colorAttachment = colorAttachment;

				} else {
					colorAttachment->resize(sceneSize);
				}

				if(depthAttachment == nullptr) {
					depthAttachment = VulkanTexture2D::create(TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | TEXTURE_USAGE_SAMPLED_BIT);

					Texture2D::AllocInfo allocInfo{};
					allocInfo.width = sceneSize.width;
					allocInfo.height = sceneSize.height;
					allocInfo.format = PixelFormat::DEPTH;
					allocInfo.mipLevels = 1;

					depthAttachment->allocate(allocInfo);

					depthAttachment->setLayout(VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

					renderTarget.depthAttachment = depthAttachment;

				} else {
					depthAttachment->resize(sceneSize);
				}
			}
		}
	}

	void VulkanFrameGraphResourcePool::clearReferences() {

		for(auto& buffers : m_Buffers) {
			for(auto& buffer : buffers) {
				buffer.useCount = 0;
			}
		}

		for(auto& textures : m_Textures) {
			for(auto& texture : textures) {
				texture.useCount = 0;
			}
		}
	}

	const Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>& VulkanFrameGraphResourcePool::getBuffers(ResourceHandle handle) const {
		auto it = findBuffer(handle);
		if(it != m_Buffers.cend()) return *it;
		throw MILO_RUNTIME_EXCEPTION(fmt::format("No buffer found with handle {}", handle));
	}

	FrameGraphBuffer VulkanFrameGraphResourcePool::getBuffer(ResourceHandle handle) {
		auto it = findBuffer(handle);
		if(it != m_Buffers.end()) {
			auto& buffers = *it;
			for(auto& buffer : buffers) {
				++buffer.useCount;
			}
			return buffers[currentSwapchainImage()];
		}
		throw MILO_RUNTIME_EXCEPTION(fmt::format("Unknown buffer with handle {}", handle));
	}

	FrameGraphBuffer VulkanFrameGraphResourcePool::getBuffer(const BufferDescription& description) {

		auto it = findBuffer(description);

		if(it == m_Buffers.cend()) {

			Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT> resources = {};

			ResourceHandle handle = ++m_ResourceHandleProvider;

			for(FrameGraphBuffer& resource : resources) {

				resource.handle = handle;
				resource.description = description;
				resource.buffer = VulkanBuffer::create(description.type);

				Buffer::AllocInfo allocInfo = {};
				allocInfo.size = description.size;

				resource.buffer->allocate(allocInfo);

				++resource.useCount;
			}

			m_Buffers.push_back(resources);
			return resources[currentSwapchainImage()];

		} else {

			auto& buffers = *it;
			for(auto& buffer : buffers) {
				++buffer.useCount;
			}

			return (*it)[currentSwapchainImage()];
		}
	}

	void VulkanFrameGraphResourcePool::destroyBuffer(ResourceHandle handle) {

		auto it = findBuffer(handle);

		if(it == m_Buffers.cend()) return;

		auto& buffers = *it;
		for(const FrameGraphBuffer& resource : buffers) {
			auto& r = const_cast<FrameGraphBuffer&>(resource);
#ifdef _DEBUG
			if(r.useCount > 0) Log::warn("Deleting buffer {} that has a use count of {}", handle, r.useCount);
#endif
			DELETE_PTR(r.buffer);
		}
		m_Buffers.erase(it);
	}

	const Array<FrameGraphTexture2D, 3>& VulkanFrameGraphResourcePool::getTextures2D(ResourceHandle handle) const {
		auto it = findTexture2D(handle);
		if(it != m_Textures.cend()) return *it;
		throw MILO_RUNTIME_EXCEPTION(fmt::format("No texture2D found with handle {}", handle));
	}

	FrameGraphTexture2D VulkanFrameGraphResourcePool::getTexture2D(ResourceHandle handle) {
		auto it = findTexture2D(handle);
		if(it != m_Textures.end()) {
			auto& textures = *it;
			for(auto& texture : textures) {
				++texture.useCount;
			}
			return textures[currentSwapchainImage()];
		}
		return {};
	}

	FrameGraphTexture2D VulkanFrameGraphResourcePool::getTexture2D(const Texture2DDescription& desc) {

		auto it = findTexture2D(desc);

		if(it == m_Textures.cend()) {

			Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT> resources = {};

			ResourceHandle handle = ++m_ResourceHandleProvider;

			for(FrameGraphTexture2D& resource : resources) {

				resource.handle = handle;
				resource.description = desc;
				resource.texture = VulkanTexture2D::create(desc.usageFlags);

				Texture2D::AllocInfo allocInfo = {};
				allocInfo.width = desc.width;
				allocInfo.height = desc.height;
				allocInfo.mipLevels = desc.mipLevels;
				allocInfo.format = desc.format;

				resource.texture->allocate(allocInfo);
				resource.texture->generateMipmaps();

				++resource.useCount;
			}

			m_Textures.push_back(resources);
			return resources[currentSwapchainImage()];

		} else {

			auto& textures = *it;
			for(auto& texture : textures) {
				++texture.useCount;
			}

			return (*it)[currentSwapchainImage()];
		}
	}

	void VulkanFrameGraphResourcePool::destroyTexture(ResourceHandle handle) {

		auto it = findTexture2D(handle);

		if(it == m_Textures.cend()) return;

		auto& textures = *it;
		for(const FrameGraphTexture2D& resource : textures) {
			auto& r = const_cast<FrameGraphTexture2D&>(resource);
#ifdef _DEBUG
			if(r.useCount > 0) Log::warn("Deleting texture {} that has a use count of {}", handle, r.useCount);
#endif
			DELETE_PTR(r.texture);
		}
		m_Textures.erase(it);
	}

	const RenderTarget& VulkanFrameGraphResourcePool::getRenderTarget(uint32_t index) const {
		if(index == UINT32_MAX) {
			index = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		}
		return m_RenderTargets[index];
	}

	void VulkanFrameGraphResourcePool::freeUnreferencedResources() {

		for(auto it = m_Buffers.begin();it != m_Buffers.end();) {
			auto& buffers = *it;
			if(buffers[0].useCount == 0) {
				for(auto& buffer : buffers) {
					DELETE_PTR(buffer.buffer);
				}
				it = m_Buffers.erase(it);
			} else {
				++it;
			}
		}

		for(auto it = m_Textures.begin();it != m_Textures.end();) {
			auto& textures = *it;
			if(textures[0].useCount == 0) {
				for(auto& texture : textures) {
					DELETE_PTR(texture.texture);
				}
				it = m_Textures.erase(it);
			} else {
				++it;
			}
		}
	}

	ArrayList<Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator VulkanFrameGraphResourcePool::findBuffer(ResourceHandle handle) const {
		for(auto it = m_Buffers.cbegin();it != m_Buffers.cend();++it) {
			const auto& resources = *it;
			if(resources[0].handle == handle) return it;
		}
		return m_Buffers.cend();
	}

	ArrayList<Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator VulkanFrameGraphResourcePool::findBuffer(const BufferDescription& desc) const {
		for(auto it = m_Buffers.cbegin();it != m_Buffers.cend();++it) {
			const auto& resources = *it;
			if(resources[0].description == desc) return it;
		}
		return m_Buffers.cend();
	}

	ArrayList<Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator VulkanFrameGraphResourcePool::findTexture2D(ResourceHandle handle) const {
		for(auto it = m_Textures.cbegin();it != m_Textures.cend();++it) {
			const auto& resources = *it;
			if(resources[0].handle == handle) return it;
		}
		return m_Textures.cend();
	}

	ArrayList<Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator VulkanFrameGraphResourcePool::findTexture2D(const Texture2DDescription& desc) const {
		for(auto it = m_Textures.cbegin();it != m_Textures.cend();++it) {
			const auto& resources = *it;
			if(resources[0].description == desc) return it;
		}
		return m_Textures.cend();
	}

	uint32_t VulkanFrameGraphResourcePool::currentSwapchainImage() {
		return VulkanContext::get()->vulkanPresenter()->currentImageIndex();
	}
}
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"
#include "milo/graphics/vulkan/presentation/VulkanPresenter.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include "milo/graphics/vulkan/buffers/VulkanBuffer.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	VulkanFrameGraphResourcePool::VulkanFrameGraphResourcePool() {
		m_Buffers.reserve(64);
		m_Textures.reserve(64);
	}

	VulkanFrameGraphResourcePool::~VulkanFrameGraphResourcePool() {
		// Resources will be automatically deleted via shared_ptr
	}

	FrameGraphBuffer VulkanFrameGraphResourcePool::getBuffer(ResourceHandle handle) {
		if(findBuffer(handle) != m_Buffers.end()) {
			return m_Buffers[handle][currentFrame()];
		}
		return {};
	}

	FrameGraphBuffer VulkanFrameGraphResourcePool::getBuffer(const BufferDescription& description) {

		auto it = findBuffer(description);

		if(it == m_Buffers.end()) {

			Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT> resources = {};

			for(FrameGraphBuffer& resource : resources) {

				resource.handle = ++m_ResourceHandleProvider;
				resource.description = description;
				resource.buffer = Shared<VulkanBuffer>(VulkanBuffer::create(description.type));

				Buffer::AllocInfo allocInfo = {};
				allocInfo.size = description.size;

				resource.buffer->allocate(allocInfo);
			}

			m_Buffers.push_back(resources);
			return resources[currentFrame()];

		} else {
			return (*it)[currentFrame()];
		}
	}

	void VulkanFrameGraphResourcePool::destroyBuffer(ResourceHandle handle) {

		auto it = findBuffer(handle);

		if(it == m_Buffers.cend()) return;

		auto& buffers = *it;
		for(FrameGraphBuffer& resource : buffers) {
			resource.buffer.reset();
		}
		m_Buffers.erase(it);
	}

	FrameGraphTexture2D VulkanFrameGraphResourcePool::getTexture2D(ResourceHandle handle) {
		if(findTexture2D(handle) != m_Textures.end()) {
			return m_Textures[handle][currentFrame()];
		}
		return {};
	}

	FrameGraphTexture2D VulkanFrameGraphResourcePool::getTexture2D(const Texture2DDescription& description) {

		auto it = findTexture2D(description);

		if(it == m_Textures.end()) {

			Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT> resources = {};

			for(FrameGraphTexture2D& resource : resources) {

				resource.handle = ++m_ResourceHandleProvider;
				resource.description = description;
				resource.texture = Shared<VulkanTexture2D>(VulkanTexture2D::create());

				Texture2D::AllocInfo allocInfo = {};
				allocInfo.width = description.width;
				allocInfo.height = description.height;
				allocInfo.mipLevels = description.mipLevels;
				allocInfo.format = description.format;

				resource.texture->allocate(allocInfo);
			}

			m_Textures.push_back(resources);
			return resources[currentFrame()];

		} else {
			return (*it)[currentFrame()];
		}
	}

	void VulkanFrameGraphResourcePool::destroyTexture(ResourceHandle handle) {

		auto it = findTexture2D(handle);

		if(it == m_Textures.cend()) return;

		auto& textures = *it;
		for(FrameGraphTexture2D& resource : textures) {
			resource.texture.reset();
		}
		m_Textures.erase(it);
	}

	void VulkanFrameGraphResourcePool::freeUnreferencedResources() {

		for(auto it = m_Buffers.begin();it != m_Buffers.end();) {
			auto& [id, resource] = *it;
			if(resource.buffer.use_count() <= 1) {
				m_Buffers.erase(it);
			} else {
				++it;
			}
		}

		for(auto it = m_Textures.begin();it != m_Textures.end();) {
			auto& [id, resource] = *it;
			if(resource.texture.use_count() <= 1) {
				m_Textures.erase(it);
			} else {
				++it;
			}
		}
	}

	ArrayList<Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT>>::iterator VulkanFrameGraphResourcePool::findBuffer(ResourceHandle handle) {
		for(auto it = m_Buffers.begin();it != m_Buffers.end();++it) {
			const auto& resources = *it;
			if(resources[0].handle == handle) return it;
		}
		return m_Buffers.end();
	}

	ArrayList<Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT>>::iterator VulkanFrameGraphResourcePool::findBuffer(const BufferDescription& desc) {
		for(auto it = m_Buffers.begin();it != m_Buffers.end();++it) {
			const auto& resources = *it;
			if(resources[0].description == desc) return it;
		}
		return m_Buffers.end();
	}

	ArrayList<Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT>>::iterator VulkanFrameGraphResourcePool::findTexture2D(ResourceHandle handle) {
		for(auto it = m_Textures.begin();it != m_Textures.end();++it) {
			const auto& resources = *it;
			if(resources[0].handle == handle) return it;
		}
		return m_Textures.end();
	}

	ArrayList<Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT>>::iterator VulkanFrameGraphResourcePool::findTexture2D(const Texture2DDescription& desc) {
		for(auto it = m_Textures.begin();it != m_Textures.end();++it) {
			const auto& resources = *it;
			if(resources[0].description == desc) return it;
		}
		return m_Textures.end();
	}

	uint32_t VulkanFrameGraphResourcePool::currentFrame() {
		return VulkanContext::get()->vulkanPresenter()->currentFrame();
	}
}
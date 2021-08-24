#pragma once

#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/graphics/vulkan/VulkanAPI.h"

namespace milo {

	class VulkanFrameGraphResourcePool : public FrameGraphResourcePool {
		friend class FrameGraphResourcePool;
	private:
		Atomic<RenderPassId> m_ResourceHandleProvider;
		ArrayList<Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT>> m_Buffers;
		ArrayList<Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT>> m_Textures;
	private:
		VulkanFrameGraphResourcePool();
		~VulkanFrameGraphResourcePool() override;
	public:
		FrameGraphBuffer getBuffer(ResourceHandle handle) override;
		FrameGraphBuffer getBuffer(const BufferDescription& description) override;
		void destroyBuffer(ResourceHandle handle) override;

		FrameGraphTexture2D getTexture2D(ResourceHandle handle) override;
		FrameGraphTexture2D getTexture2D(const Texture2DDescription& description) override;
		void destroyTexture(ResourceHandle handle) override;

		void freeUnreferencedResources() override;
	private:
		ArrayList<Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT>>::iterator findBuffer(ResourceHandle handle);
		ArrayList<Array<FrameGraphBuffer, MAX_FRAMES_IN_FLIGHT>>::iterator findBuffer(const BufferDescription& desc);
		ArrayList<Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT>>::iterator findTexture2D(ResourceHandle handle);
		ArrayList<Array<FrameGraphTexture2D, MAX_FRAMES_IN_FLIGHT>>::iterator findTexture2D(const Texture2DDescription& desc);
	private:
		static uint32_t currentFrame();
	};

}
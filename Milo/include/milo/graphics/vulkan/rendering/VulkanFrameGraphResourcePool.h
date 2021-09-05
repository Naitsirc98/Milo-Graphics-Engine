#pragma once

#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/graphics/vulkan/VulkanAPI.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

namespace milo {

	class VulkanFrameGraphResourcePool : public FrameGraphResourcePool {
		friend class FrameGraphResourcePool;
	private:
		Atomic<RenderPassId> m_ResourceHandleProvider;
		ArrayList<Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>> m_Buffers;
		ArrayList<Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>> m_Textures;
		Array<RenderTarget, MAX_SWAPCHAIN_IMAGE_COUNT> m_RenderTargets{};
	private:
		VulkanFrameGraphResourcePool();
		~VulkanFrameGraphResourcePool() override;
	public:
		void update() override;

		void clearReferences() override;

		const Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>& getBuffers(ResourceHandle handle) const;
		FrameGraphBuffer getBuffer(ResourceHandle handle) override;
		FrameGraphBuffer getBuffer(const BufferDescription& description) override;
		void destroyBuffer(ResourceHandle handle) override;

		const Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>& getTextures2D(ResourceHandle handle) const;
		FrameGraphTexture2D getTexture2D(ResourceHandle handle) override;
		FrameGraphTexture2D getTexture2D(const Texture2DDescription& desc) override;
		void destroyTexture(ResourceHandle handle) override;

		const RenderTarget& getRenderTarget(uint32_t index = UINT32_MAX) const;

		void freeUnreferencedResources() override;
	private:
		ArrayList<Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator findBuffer(ResourceHandle handle) const;
		ArrayList<Array<FrameGraphBuffer, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator findBuffer(const BufferDescription& desc) const;
		ArrayList<Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator findTexture2D(ResourceHandle handle) const;
		ArrayList<Array<FrameGraphTexture2D, MAX_SWAPCHAIN_IMAGE_COUNT>>::const_iterator findTexture2D(const Texture2DDescription& desc) const;
	private:
		static uint32_t currentSwapchainImage();
	};

}
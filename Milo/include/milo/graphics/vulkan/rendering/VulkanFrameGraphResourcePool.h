#pragma once

#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/graphics/vulkan/VulkanAPI.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/buffers/VulkanFramebuffer.h"

namespace milo {

	class VulkanFrameGraphResourcePool : public FrameGraphResourcePool {
		friend class FrameGraphResourcePool;
	private:
		VulkanFrameGraphResourcePool();
		~VulkanFrameGraphResourcePool() override;
	protected:
		uint32_t currentFramebufferIndex() const override;
		uint32_t maxDefaultFramebuffersCount() const override;
	};

}
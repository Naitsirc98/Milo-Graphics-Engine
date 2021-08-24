#pragma once

#include "milo/graphics/rendering/passes/FinalRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanFinalRenderPass : public FinalRenderPass {
		friend class FinalRenderPass;
	public:
		VulkanFinalRenderPass();
		~VulkanFinalRenderPass();
		void compile() override;
		void execute(Scene* scene) override;
	};

}
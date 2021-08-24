#pragma once

#include "milo/graphics/rendering/passes/GeometryRenderPass.h"
#include "milo/graphics/vulkan/VulkanDevice.h"

namespace milo {

	class VulkanGeometryRenderPass : public GeometryRenderPass {
		friend class GeometryRenderPass;
	public:
		VulkanGeometryRenderPass();
		~VulkanGeometryRenderPass();
		void compile() override;
		void execute(Scene* scene) override;
	};

}

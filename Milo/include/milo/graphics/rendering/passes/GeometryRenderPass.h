#pragma once

#include <milo/scenes/Scene.h>
#include "RenderPass.h"

namespace milo {

	class GeometryRenderPass : public RenderPass {
	public:
		GeometryRenderPass() = default;
		virtual ~GeometryRenderPass() override = default;
		InputDescription inputDescription() const override;
		OutputDescription outputDescription() const override;
		RenderPassId getId() const override;
	public:
		static GeometryRenderPass* create();
		static size_t id();
	};

}
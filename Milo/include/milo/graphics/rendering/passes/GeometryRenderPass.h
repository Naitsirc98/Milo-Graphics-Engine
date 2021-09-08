#pragma once

#include <milo/scenes/Scene.h>
#include "RenderPass.h"

namespace milo {

	class GeometryRenderPass : public RenderPass {
	public:
		GeometryRenderPass() = default;
		virtual ~GeometryRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static GeometryRenderPass* create();
		static size_t id();
	};

}
#pragma once

#include "RenderPass.h"

namespace milo {

	class BoundingVolumeRenderPass : public RenderPass {
	public:
		BoundingVolumeRenderPass() = default;
		virtual ~BoundingVolumeRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static BoundingVolumeRenderPass* create();
		static size_t id();
	};

}
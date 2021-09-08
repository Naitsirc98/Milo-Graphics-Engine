#pragma once

#include "milo/scenes/Scene.h"
#include "RenderPass.h"
#include "milo/assets/skybox/Skybox.h"

namespace milo {

	class SkyboxRenderPass : public RenderPass {
	public:
		SkyboxRenderPass() = default;
		virtual ~SkyboxRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static SkyboxRenderPass* create();
		static size_t id();
	};
}
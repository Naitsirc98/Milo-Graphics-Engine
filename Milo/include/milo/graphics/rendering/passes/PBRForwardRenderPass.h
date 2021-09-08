#pragma once

#include <milo/graphics/rendering/descriptions/ResourceDescriptions.h>
#include "milo/assets/materials/Material.h"
#include "milo/assets/meshes/Mesh.h"
#include "RenderPass.h"

namespace milo {

	class PBRForwardRenderPass : public RenderPass {
	public:
		PBRForwardRenderPass() = default;
		virtual ~PBRForwardRenderPass() override = default;
		RenderPassId getId() const override;
		const String& name() const override;
	public:
		static PBRForwardRenderPass* create();
		static size_t id();
	};
}
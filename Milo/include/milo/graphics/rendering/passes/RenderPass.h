#pragma once

#include "milo/graphics/rendering/descriptions/ResourceDescriptions.h"
#include "milo/scenes/Scene.h"

#define DEFINE_RENDER_PASS_ID(renderPassName) static const size_t id = std::hash<String>()(#renderPassName)
#define MAX_RENDER_PASS_TEXTURES 16
#define MAX_RENDER_PASS_BUFFERS 16

namespace milo {

	using RenderPassId = size_t;

	class RenderPass {
		friend class FrameGraph;
	public:
		struct DependenciesDescription {
			BufferDescription buffers[MAX_RENDER_PASS_BUFFERS]{};
			uint8_t bufferCount = 0;
			Texture2DDescription textures[MAX_RENDER_PASS_TEXTURES]{};
			uint8_t textureCount = 0;
		};
		using InputDescription = DependenciesDescription;
		using OutputDescription = DependenciesDescription;

		struct Dependencies {
			FrameGraphBuffer buffers[MAX_RENDER_PASS_BUFFERS]{};
			uint8_t bufferCount = 0;
			FrameGraphTexture2D textures[MAX_RENDER_PASS_TEXTURES]{};
			uint8_t textureCount = 0;
		};
		using Input = Dependencies;
		using Output = Dependencies;

	protected:
		Input m_Input = {};
		Output m_Output = {};
	public:
		virtual ~RenderPass() = default;
		virtual RenderPassId getId() const = 0;
		virtual RenderPass::InputDescription inputDescription() const = 0;
		virtual RenderPass::OutputDescription outputDescription() const = 0;
		virtual void compile() = 0;
		virtual void execute(Scene* scene) = 0;
	};
}
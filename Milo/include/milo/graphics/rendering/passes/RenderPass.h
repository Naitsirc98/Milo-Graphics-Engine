#pragma once

#include "milo/graphics/rendering/descriptions/ResourceDescriptions.h"
#include "milo/scenes/Scene.h"

#define DEFINE_RENDER_PASS_ID(renderPassName) static const size_t id = std::hash<String>()(#renderPassName)

namespace milo {

	using RenderPassId = size_t;

	class RenderPass {
		friend class FrameGraph;
	public:
		enum class LoadOp {
			Undefined, Clear, Load
		};
		struct Attachment {
			PixelFormat format{PixelFormat::RGBA32F};
			uint32_t samples{1};
			LoadOp loadOp{LoadOp::Undefined};
		};
		struct Description {
			ArrayList<Attachment> colorAttachments;
			Optional<Attachment> depthAttachment;
		};
	public:
		virtual ~RenderPass() = default;
		virtual RenderPassId getId() const = 0;
		virtual bool shouldCompile(Scene* scene) const = 0;
		virtual void compile(Scene* scene, FrameGraphResourcePool* resourcePool) = 0;
		virtual void execute(Scene* scene) = 0;
		virtual const String& name() const = 0;
	};
}
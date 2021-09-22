#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/time/Profiler.h"

namespace milo {

	FrameGraph::FrameGraph() {
		m_RenderPasses.reserve(16);
		m_RenderPassExecutionList.reserve(m_RenderPasses.capacity());
	}

	FrameGraph::~FrameGraph() {
		for(RenderPass* renderPass : m_RenderPasses) {
			DELETE_PTR(renderPass);
		}
	}

	void FrameGraph::init(FrameGraphResourcePool* resourcePool) {
		m_ResourcePool = resourcePool;
	}

	void FrameGraph::setup(Scene* scene) {

		MILO_PROFILE_FUNCTION;

		const WorldRenderer& renderer = WorldRenderer::get();

		push<PreDepthRenderPass>();
		push<LightCullingPass>();
		push<ShadowMapRenderPass>();
		//push<GeometryRenderPass>();
		push<PBRForwardRenderPass>();

		if(renderer.lights().skybox != nullptr) {
			push<SkyboxRenderPass>();
		}

		if(renderer.showBoundingVolumes()) {
			push<BoundingVolumeRenderPass>();
		}

		if(renderer.showGrid()) {
			push<GridRenderPass>();
		}

		if(getSimulationState() != SimulationState::Editor) {
			push<FinalRenderPass>();
		}
	}

	void FrameGraph::compile(Scene* scene) {
		MILO_PROFILE_FUNCTION;
		m_ResourcePool->compile(scene);
		for(RenderPass* pass : m_RenderPassExecutionList) {
			if(pass->shouldCompile(scene)) {
				Log::debug("Compiling {}", pass->name());
				pass->compile(scene, m_ResourcePool);
			}
		}
		deleteUnusedRenderPasses();
	}

	void FrameGraph::execute(Scene* scene) {

		MILO_PROFILE_FUNCTION;

		for(RenderPass* renderPass : m_RenderPassExecutionList) {
			renderPass->execute(scene);
		}
		m_RenderPassExecutionList.clear();
	}

	static constexpr uint32_t MAX_RENDER_PASS_UNUSED_COUNT = 600;

	inline void FrameGraph::deleteUnusedRenderPasses() {

		for(auto it = m_RenderPasses.begin();it != m_RenderPasses.end();) {

			const RenderPass* renderPass = *it;

			bool unused = true;
			for(RenderPass* activeRenderPass : m_RenderPassExecutionList) {
				if(activeRenderPass->getId() == renderPass->getId()) {
					unused = false;
					break;
				}
			}

			if(unused) {
				uint32_t& count = m_RenderPassUnusedCount[renderPass->getId()];
				if(++count >= MAX_RENDER_PASS_UNUSED_COUNT) {
					DELETE_PTR(renderPass);
					m_RenderPasses.erase(it);
					m_RenderPassUnusedCount.erase(renderPass->getId());
				}
			} else {
				++it;
			}
		}
	}
}
#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"

namespace milo {

	FrameGraph::FrameGraph() {
		m_RenderPasses.reserve(64);
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

		push<GeometryRenderPass>();

		//push<PBRForwardRenderPass>();

		if(scene->skybox() != nullptr) {
			push<SkyboxRenderPass>();
		}

		//push<FinalRenderPass>();
	}

	void FrameGraph::compile(Scene* scene) {
		m_ResourcePool->compile(scene);
		for(RenderPass* pass : m_RenderPassExecutionList) {
			if(pass->shouldCompile(scene)) {
				pass->compile(scene, m_ResourcePool);
			}
		}
		deleteUnusedRenderPasses();
	}

	void FrameGraph::execute(Scene* scene) {

		for(RenderPass* renderPass : m_RenderPassExecutionList) {
			renderPass->execute(scene);
		}
		m_RenderPassExecutionList.clear();
	}

	inline void FrameGraph::deleteUnusedRenderPasses() {
		std::remove_if(m_RenderPasses.begin(), m_RenderPasses.end(), [&](RenderPass* renderPass) {

			for(RenderPass* activeRenderPass : m_RenderPassExecutionList) {
				if(activeRenderPass->getId() == renderPass->getId()) return false;
			}

			DELETE_PTR(renderPass);
			return true;
		});
	}
}
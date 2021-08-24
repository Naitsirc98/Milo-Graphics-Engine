#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	WorldRenderer::WorldRenderer() {

		m_GraphicsPresenter = GraphicsPresenter::get();

		m_ResourcePool = FrameGraphResourcePool::create();

		m_FrameGraph.init(m_ResourcePool);
	}

	WorldRenderer::~WorldRenderer() {
		m_GraphicsPresenter = nullptr;
		DELETE_PTR(m_ResourcePool);
	}

	void WorldRenderer::render(Scene* scene) {

		if(m_GraphicsPresenter->begin()) {

			m_FrameGraph.setup(scene);
			m_FrameGraph.compile(scene);
			m_FrameGraph.execute(scene);

			m_GraphicsPresenter->end();
		}
	}

	void WorldRenderer::render() {
		s_Instance->render(SceneManager::activeScene());
	}

	WorldRenderer* WorldRenderer::s_Instance = nullptr;

	void WorldRenderer::init() {
		s_Instance = new WorldRenderer();
	}

	void WorldRenderer::shutdown() {
		DELETE_PTR(s_Instance);
	}
}
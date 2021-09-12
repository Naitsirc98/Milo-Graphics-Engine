#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"

namespace milo {

	WorldRenderer::WorldRenderer() {

		m_GraphicsPresenter = GraphicsPresenter::get();

		m_ResourcePool = FrameGraphResourcePool::create();
		m_ResourcePool->init();

		m_FrameGraph.init(m_ResourcePool);

		m_DrawCommands.reserve(1024);
		m_ShadowDrawCommands.reserve(1024);

		//m_ShowGrid = getSimulationState() == SimulationState::Editor;
	}

	WorldRenderer::~WorldRenderer() {
		m_GraphicsPresenter = nullptr;
		DELETE_PTR(m_ResourcePool);
	}

	void WorldRenderer::render(Scene* scene) {
		m_FrameGraph.setup(scene);
		m_FrameGraph.compile(scene);
		m_FrameGraph.execute(scene);
	}

	void WorldRenderer::render() {
		s_Instance->render(SceneManager::activeScene());
	}

	void WorldRenderer::update() {
		Scene* scene = SceneManager::activeScene();
		generateDrawCommands(scene);
	}

	void WorldRenderer::generateDrawCommands(Scene* scene) {

		auto& drawCommands = s_Instance->m_DrawCommands;
		auto& shadowsDrawCommands = s_Instance->m_ShadowDrawCommands;

		drawCommands.clear();
		shadowsDrawCommands.clear();

		auto components = scene->group<Transform, MeshView>();
		for(EntityId entityId : components) {

			const Transform& transform = components.get<Transform>(entityId);
			const MeshView& meshView = components.get<MeshView>(entityId);
			if(meshView.mesh == nullptr || meshView.material == nullptr) continue;

			DrawCommand drawCommand;
			drawCommand.transform = transform.modelMatrix();
			drawCommand.mesh = meshView.mesh;
			drawCommand.material = meshView.material;

			drawCommands.push_back(drawCommand);
			shadowsDrawCommands.push_back(drawCommand);
		}

		ArrayList<int> l = {1, 2, 5, 2, 1, 5, 67, 8};

		std::sort(l.begin(), l.end());

		std::sort(drawCommands.begin(), drawCommands.end());
		std::sort(shadowsDrawCommands.begin(), shadowsDrawCommands.end());
	}

	Framebuffer& WorldRenderer::getFramebuffer() const {
		return *m_ResourcePool->getDefaultFramebuffer();
	}

	bool WorldRenderer::showGrid() const {
		return m_ShowGrid;
	}

	void WorldRenderer::setShowGrid(bool show) {
		m_ShowGrid = show;
	}

	bool WorldRenderer::shadowsEnabled() const {
		return m_ShadowsEnabled;
	}

	void WorldRenderer::setShadowsEnabled(bool shadowsEnabled) {
		m_ShadowsEnabled = shadowsEnabled;
	}

	void WorldRenderer::submit(DrawCommand drawCommand, bool castShadows) {
		m_DrawCommands.push_back(drawCommand);
		if(castShadows) m_ShadowDrawCommands.push_back(drawCommand);
	}

	const ArrayList<DrawCommand>& WorldRenderer::drawCommands() const {
		return m_DrawCommands;
	}

	const ArrayList<DrawCommand>& WorldRenderer::shadowsDrawCommands() const {
		return m_ShadowDrawCommands;
	}

	WorldRenderer* WorldRenderer::s_Instance = nullptr;

	WorldRenderer& WorldRenderer::get() {
		return *s_Instance;
	}

	void WorldRenderer::init() {
		s_Instance = new WorldRenderer();
	}

	void WorldRenderer::shutdown() {
		DELETE_PTR(s_Instance);
	}
}
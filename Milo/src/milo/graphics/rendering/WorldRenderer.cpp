#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/editor/MiloEditor.h"

namespace milo {

	WorldRenderer::WorldRenderer() {

		m_GraphicsPresenter = GraphicsPresenter::get();

		m_ResourcePool = FrameGraphResourcePool::create();
		m_ResourcePool->init();

		m_FrameGraph.init(m_ResourcePool);

		m_DrawCommands.reserve(8192);
		m_ShadowDrawCommands.reserve(8192);

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

		Polyhedron frustum;

		if(getSimulationState() == SimulationState::Editor) {
			const auto& camera = MiloEditor::camera();
			float fov, znear, zfar;
			decomposeProjectionMatrix(camera.projMatrix(), fov, znear, zfar);
			frustum = buildFrustumPolyhedron(camera.viewMatrix(), fov, camera.aspect(), znear, zfar);
		} else {
			const auto* camera = scene->camera();
			float fov, znear, zfar;
			decomposeProjectionMatrix(camera->projectionMatrix(), fov, znear, zfar);
			float aspect = camera->viewport().x / camera->viewport().y;
			Matrix4 viewMatrix = camera->viewMatrix(scene->cameraEntity().getComponent<Transform>().translation);
			frustum = buildFrustumPolyhedron(viewMatrix, fov, aspect, znear, zfar);
		}

		auto components = scene->group<Transform, MeshView>();
		for(EntityId entityId : components) {

			const Transform& transform = components.get<Transform>(entityId);
			const MeshView& meshView = components.get<MeshView>(entityId);

			Matrix4 modelMatrix = transform.modelMatrix();

			if(meshView.mesh == nullptr || meshView.material == nullptr) continue;
			//if(!meshView.mesh->boundingVolume().isVisible(modelMatrix, frustum.plane, 6)) continue; // TODO

			DrawCommand drawCommand;
			drawCommand.transform = modelMatrix;
			drawCommand.mesh = meshView.mesh;
			drawCommand.material = meshView.material;

			drawCommands.push_back(drawCommand);
			shadowsDrawCommands.push_back(drawCommand);
		}

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
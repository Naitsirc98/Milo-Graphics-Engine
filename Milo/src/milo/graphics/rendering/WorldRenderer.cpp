#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/editor/MiloEditor.h"
#include <algorithm>

namespace milo {

	WorldRenderer::WorldRenderer() {

		m_GraphicsPresenter = GraphicsPresenter::get();

		m_ResourcePool = FrameGraphResourcePool::create();
		m_ResourcePool->init();

		m_FrameGraph.init(m_ResourcePool);

		m_DrawCommands.reserve(8192);
		m_ShadowDrawCommands.reserve(8192);

		m_LightEnvironment.pointLights.reserve(1024);

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

		getCameraInfo(scene);
		generateLightEnvironment(scene);

		const CameraInfo& camera = s_Instance->camera();

		auto components = scene->group<Transform, MeshView>();
		for(EntityId entityId : components) {

			const Transform& transform = components.get<Transform>(entityId);
			const MeshView& meshView = components.get<MeshView>(entityId);

			Matrix4 modelMatrix = transform.modelMatrix();

			Mesh* mesh = meshView.mesh;
			Material* material = meshView.material;
			if(mesh == nullptr || material == nullptr) continue;
			if(mesh->canBeCulled() && !mesh->boundingVolume().isVisible(modelMatrix, camera.frustum.plane, 6)) continue; // TODO

			DrawCommand drawCommand;
			drawCommand.transform = modelMatrix;
			drawCommand.mesh = mesh;
			drawCommand.material = material;

			drawCommands.push_back(drawCommand);
			shadowsDrawCommands.push_back(drawCommand);
		}

		std::sort(drawCommands.begin(), drawCommands.end());
		std::sort(shadowsDrawCommands.begin(), shadowsDrawCommands.end());
	}

	void WorldRenderer::getCameraInfo(Scene* scene) {
		CameraInfo& c = s_Instance->m_Camera;
		if(getSimulationState() == SimulationState::Editor) {
			const auto& camera = MiloEditor::camera();
			float fov, znear, zfar;
			decomposeProjectionMatrix(camera.projMatrix(), fov, znear, zfar);
			c.proj = camera.projMatrix();
			c.view = camera.viewMatrix();
			c.projView = c.proj * c.view;
			c.frustum = buildFrustumPolyhedron(camera.viewMatrix(), fov, camera.aspect(), znear, zfar);
		} else {
			const auto* camera = scene->camera();
			float fov, znear, zfar;
			decomposeProjectionMatrix(camera->projectionMatrix(), fov, znear, zfar);
			float aspect = camera->viewport().x / camera->viewport().y;
			Matrix4 viewMatrix = camera->viewMatrix(scene->cameraEntity().getComponent<Transform>().translation);
			c.proj = camera->projectionMatrix();
			c.view = viewMatrix;
			c.projView = c.proj * c.view;
			c.frustum = buildFrustumPolyhedron(viewMatrix, fov, aspect, znear, zfar);
		}
	}

	void WorldRenderer::generateLightEnvironment(Scene* scene) {

		const CameraInfo& camera = s_Instance->camera();
		LightEnvironment& env = s_Instance->m_LightEnvironment;

		ECSComponentView<SkyLight> skyLight = scene->view<SkyLight>();
		if(!skyLight.empty()) {
			EntityId entity = *skyLight.begin();
			SkyLight sky = skyLight.get<SkyLight>(entity);
			env.dirLight = sky.light;
			env.skybox = sky.sky;
		} else {
			ECSComponentView<DirectionalLight> dirLight = scene->view<DirectionalLight>();
			if(!dirLight.empty()) {
				EntityId entity = *dirLight.begin();
				env.dirLight = dirLight.get<DirectionalLight>(entity);
			}
			if(scene->skyboxView() != nullptr) env.skybox = scene->skyboxView()->skybox;
		}

		// FIXME
		//env.pointLights.clear();
		//auto components = scene->group<Transform, PointLight>();
		//for(EntityId entityId : components) {
		//	const auto& transform = components.get<Transform>(entityId);
		//	PointLight pointLight = components.get<PointLight>(entityId);
		//	pointLight.position = transform.translation;
		//	env.pointLights.push_back(pointLight);
		//}
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

	bool WorldRenderer::showBoundingVolumes() const {
		return m_ShowBoundingVolumes;
	}

	void WorldRenderer::setShowBoundingVolumes(bool value) {
		m_ShowBoundingVolumes = value;
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

	const CameraInfo& WorldRenderer::camera() const {
		return m_Camera;
	}

	const LightEnvironment& WorldRenderer::lights() const {
		return m_LightEnvironment;
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
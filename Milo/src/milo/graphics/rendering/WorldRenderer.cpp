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
		generateDrawCommands(scene);
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
			c.position = camera.position();
		} else {
			const auto* camera = scene->camera();
			float fov, znear, zfar;
			decomposeProjectionMatrix(camera->projectionMatrix(), fov, znear, zfar);
			float aspect = camera->viewport().x / camera->viewport().y;
			Vector3 position = scene->cameraEntity().getComponent<Transform>().translation();
			Matrix4 viewMatrix = camera->viewMatrix(position);
			c.proj = camera->projectionMatrix();
			c.view = viewMatrix;
			c.projView = c.proj * c.view;
			c.frustum = buildFrustumPolyhedron(viewMatrix, fov, aspect, znear, zfar);
			c.position = position;
		}
	}

	void WorldRenderer::generateLightEnvironment(Scene* scene) {

		const CameraInfo& camera = s_Instance->camera();
		LightEnvironment& env = s_Instance->m_LightEnvironment;

		bool dirLightPresent = false;

		ECSComponentView<SkyLight> skyLight = scene->view<SkyLight>();
		if(!skyLight.empty()) {
			EntityId entity = *skyLight.begin();
			SkyLight sky = skyLight.get<SkyLight>(entity);
			env.dirLight = sky.light;
			env.skybox = sky.sky;
			dirLightPresent = true;
		} else {
			ECSComponentView<DirectionalLight> dirLight = scene->view<DirectionalLight>();
			if(!dirLight.empty()) {
				EntityId entity = *dirLight.begin();
				env.dirLight = dirLight.get<DirectionalLight>(entity);
				dirLightPresent = true;
			}
			if(scene->skyboxView() != nullptr) env.skybox = scene->skyboxView()->skybox;
		}

		if(dirLightPresent) {
			calculateShadowCascades(scene);
		}

		env.pointLights.clear();
		auto components = scene->view<Transform>() | scene->view<PointLight>();
		for(EntityId entityId : components) {
			const Transform& transform = components.get<Transform>(entityId);
			PointLight& pointLight = components.get<PointLight>(entityId);
			pointLight.position = Vector4(transform.translation(), 1.0f);
			env.pointLights.push_back(pointLight);
		}
	}

	void WorldRenderer::calculateShadowCascades(Scene* scene) {

		static const float CascadeFarPlaneOffset = 50.0f;
		static const float CascadeNearPlaneOffset = -50.0f;

		float fov, zNear, zFar;
		decomposeProjectionMatrix(s_Instance->m_Camera.proj, fov, zNear, zFar);
		zFar = std::min(zFar, s_Instance->m_ShadowsMaxDistance);
		float zRange = zFar - zNear;

		const DirectionalLight& light = s_Instance->m_LightEnvironment.dirLight.value();

		const Matrix4 projView = s_Instance->m_Camera.projView;
		const Matrix4 invProjView = glm::inverse(projView);

		auto& shadowCascades = s_Instance->m_ShadowCascades;

		float cascadeRanges[5]{0.0f};
		calculateShadowCascadeRanges(cascadeRanges, zNear, zFar);

		float lastSplitDist = 0.0f;

		for(uint32_t index = 0; index < shadowCascades.size(); ++index) {

			ShadowCascade& cascade = shadowCascades[index];

			float splitDist = cascadeRanges[index];

			Vector3 frustumCorners[8] = {
					Vector3(-1.0f,  1.0f, -1.0f),
					Vector3(1.0f,  1.0f, -1.0f),
					Vector3(1.0f, -1.0f, -1.0f),
					Vector3(-1.0f, -1.0f, -1.0f),
					Vector3(-1.0f,  1.0f,  1.0f),
					Vector3(1.0f,  1.0f,  1.0f),
					Vector3(1.0f, -1.0f,  1.0f),
					Vector3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			for (uint32_t i = 0; i < 8; i++) {
				Vector4 invCorner = invProjView * Vector4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++) {
				Vector3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			Vector3 frustumCenter = Vector3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;

			//frustumCenter *= 0.01f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			Vector3 maxExtents = Vector3(radius);
			Vector3 minExtents = -maxExtents;

			Vector3 lightDir = -light.direction;
			Matrix4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, Vector3(0.0f, 0.0f, 1.0f));
			Matrix4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + CascadeNearPlaneOffset, maxExtents.z - minExtents.z + CascadeFarPlaneOffset);

			// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
			Matrix4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			const float ShadowMapResolution = 4096.0f;
			Vector4 shadowOrigin = (shadowMatrix * Vector4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
			Vector4 roundedOrigin = glm::round(shadowOrigin);
			Vector4 roundOffset = roundedOrigin - shadowOrigin;
			roundOffset = roundOffset * 2.0f / ShadowMapResolution;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			lightOrthoMatrix[3] += roundOffset;

			// Store split distance and matrix in cascade
			cascade.splitDepth = (zNear + splitDist * zRange) * -1.0f;
			cascade.viewProj = lightOrthoMatrix * lightViewMatrix;
			cascade.view = lightViewMatrix;

			lastSplitDist = cascadeRanges[index];
		}
	}

	void WorldRenderer::calculateShadowCascadeRanges(float* cascadeRanges, float zNear, float zFar) {

		float splitFactor = 0.75f;

		const int32_t cascadesCount = (int32_t)s_Instance->m_ShadowCascades.size();
		for(int32_t i = 1;i < cascadesCount;++i) {

			float inv = (float)i / (float)cascadesCount;
			float a = zNear + (inv * (zFar - zNear));
			float b = zNear * powf(zFar / zNear, inv);
			float z = lerp(a, b, splitFactor);

			cascadeRanges[i] = z;
		}

		cascadeRanges[0] = zNear;
		cascadeRanges[cascadesCount] = zFar;
	}

	FrameGraphResourcePool& WorldRenderer::resources() const {
		return *m_ResourcePool;
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

	float WorldRenderer::shadowsMaxDistance() const {
		return m_ShadowsMaxDistance;
	}

	void WorldRenderer::setShadowsMaxDistance(float distance) {
		m_ShadowsMaxDistance = distance;
	}

	const Size& WorldRenderer::shadowsMapSize() const {
		return m_ShadowsMapSize;
	}

	void WorldRenderer::setShadowsMapSize(const Size& size) {
		m_ShadowsMapSize = size;
	}

	const Array<ShadowCascade, 4>& WorldRenderer::shadowCascades() const {
		return m_ShadowCascades;
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
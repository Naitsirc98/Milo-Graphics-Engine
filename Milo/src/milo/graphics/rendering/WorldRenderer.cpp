#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/editor/MiloEditor.h"
#include <algorithm>
#include "milo/time/Profiler.h"

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

		MILO_PROFILE_FUNCTION;

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
			if(false && mesh->canBeCulled() && !mesh->boundingVolume().isVisible(modelMatrix, camera.frustum.plane, 6)) continue; // TODO

			DrawCommand drawCommand;
			drawCommand.transform = modelMatrix;
			drawCommand.mesh = mesh;
			drawCommand.material = material;

			drawCommands.push_back(drawCommand);

			if(meshView.castShadows) {
				shadowsDrawCommands.push_back(drawCommand);
			}
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
			c.aspect = camera.aspect();
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
			c.aspect = aspect;
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

		static const float CascadeNearPlaneOffset = -50.0f;
		static const float CascadeFarPlaneOffset = 50.0f;
		static const float CascadeSplitLambda = 0.92f;

		auto viewProjection = s_Instance->camera().projView;

		const int SHADOW_MAP_CASCADE_COUNT = 4;
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

		float fov, zNear, zFar;
		decomposeProjectionMatrix(s_Instance->camera().proj, fov, zNear, zFar);

		float zRange = zFar - zNear;

		float minZ = zNear;
		float maxZ = zNear + zRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		auto& cascades = s_Instance->m_ShadowCascades;

		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			float p = (i + 1.0f) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = CascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - zNear) / zRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
			float splitDist = cascadeSplits[i];

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
			Matrix4 invCam = glm::inverse(viewProjection);
			for (uint32_t i = 0; i < 8; i++) {
				Vector4 invCorner = invCam * Vector4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++) {
				Vector3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			Vector3 frustumCenter = Vector3(0.0f);
			for (uint32_t i = 0; i < 8; i++)
				frustumCenter += frustumCorners[i];

			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			Vector3 maxExtents = Vector3(radius);
			Vector3 minExtents = -maxExtents;

			Vector3 lightDir = -glm::normalize(s_Instance->m_LightEnvironment.dirLight->direction);
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
			cascades[i].splitDepth = (zNear + splitDist * zRange) * -1.0f;
			cascades[i].viewProj = lightOrthoMatrix * lightViewMatrix;
			cascades[i].view = lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
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

	bool WorldRenderer::showShadowCascades() const {
		return m_ShowShadowCascades;
	}

	void WorldRenderer::setShowShadowCascades(bool value) {
		m_ShowShadowCascades = value;
	}

	bool WorldRenderer::softShadows() const {
		return m_SoftShadows;
	}

	void WorldRenderer::setSoftShadows(bool value) {
		m_SoftShadows = value;
	}

	bool WorldRenderer::shadowCascadeFading() const {
		return m_ShadowCascadeFading;
	}

	void WorldRenderer::setShadowCascadeFading(bool value) {
		m_ShadowCascadeFading = value;
	}

	bool WorldRenderer::shadowCascadeFadingValue() const {
		return m_CascadeFading;
	}

	void WorldRenderer::setShadowCascadeFadingValue(float value) {
		m_CascadeFading = value;
	}

	bool WorldRenderer::useMultithreading() const {
		return m_UseMultithreading;
	}

	void WorldRenderer::setUseMultithreading(bool useMultithreading) {
		m_UseMultithreading = useMultithreading;
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
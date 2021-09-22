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
			if(mesh->canBeCulled() && !mesh->boundingVolume().isVisible(modelMatrix, camera.frustum.plane, 6)) continue; // TODO

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

		// TODO: less hard-coding!
		float nearClip = 0.1f;
		float farClip = 1000.0f;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		auto& cascades = s_Instance->m_ShadowCascades;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float p = (i + 1.0f) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = CascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
					glm::vec3(-1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f,  1.0f, -1.0f),
					glm::vec3(1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f, -1.0f, -1.0f),
					glm::vec3(-1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f,  1.0f,  1.0f),
					glm::vec3(1.0f, -1.0f,  1.0f),
					glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(viewProjection);
			for (uint32_t i = 0; i < 8; i++)
			{
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++)
			{
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++)
				frustumCenter += frustumCorners[i];

			frustumCenter /= 8.0f;

			//frustumCenter *= 0.1f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++)
			{
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = -glm::normalize(s_Instance->m_LightEnvironment.dirLight->direction);
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + CascadeNearPlaneOffset, maxExtents.z - minExtents.z + CascadeFarPlaneOffset);

			// Offset to texel space to avoid shimmering (from https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering)
			glm::mat4 shadowMatrix = lightOrthoMatrix * lightViewMatrix;
			const float ShadowMapResolution = 4096.0f;
			glm::vec4 shadowOrigin = (shadowMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) * ShadowMapResolution / 2.0f;
			glm::vec4 roundedOrigin = glm::round(shadowOrigin);
			glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
			roundOffset = roundOffset * 2.0f / ShadowMapResolution;
			roundOffset.z = 0.0f;
			roundOffset.w = 0.0f;

			lightOrthoMatrix[3] += roundOffset;

			// Store split distance and matrix in cascade
			cascades[i].splitDepth = (nearClip + splitDist * clipRange) * -1.0f;
			cascades[i].viewProj = lightOrthoMatrix * lightViewMatrix;
			cascades[i].view = lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
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

	Vector3 WorldRenderer::getFrustumCorner(const Matrix4& m, uint32_t corner) {
		float d1, d2, d3;
		float n1x, n1y, n1z, n2x, n2y, n2z, n3x, n3y, n3z;
		switch (corner) {
			case 0: // left, bottom, near
				n1x = m[0][3] + m[0][0];
				n1y = m[1][3] + m[1][0];
				n1z = m[2][3] + m[2][0];
				d1 = m[3][3] + m[3][0]; // left
				n2x = m[0][3] + m[0][1];
				n2y = m[1][3] + m[1][1];
				n2z = m[2][3] + m[2][1];
				d2 = m[3][3] + m[3][1]; // bottom
				n3x = m[0][3] + m[0][2];
				n3y = m[1][3] + m[1][2];
				n3z = m[2][3] + m[2][2];
				d3 = m[3][3] + m[3][2]; // near
				break;
			case 1: // right, bottom, near
				n1x = m[0][3] - m[0][0];
				n1y = m[1][3] - m[1][0];
				n1z = m[2][3] - m[2][0];
				d1 = m[3][3] - m[3][0]; // right
				n2x = m[0][3] + m[0][1];
				n2y = m[1][3] + m[1][1];
				n2z = m[2][3] + m[2][1];
				d2 = m[3][3] + m[3][1]; // bottom
				n3x = m[0][3] + m[0][2];
				n3y = m[1][3] + m[1][2];
				n3z = m[2][3] + m[2][2];
				d3 = m[3][3] + m[3][2]; // near
				break;
			case 2: // right, top, near
				n1x = m[0][3] - m[0][0];
				n1y = m[1][3] - m[1][0];
				n1z = m[2][3] - m[2][0];
				d1 = m[3][3] - m[3][0]; // right
				n2x = m[0][3] - m[0][1];
				n2y = m[1][3] - m[1][1];
				n2z = m[2][3] - m[2][1];
				d2 = m[3][3] - m[3][1]; // top
				n3x = m[0][3] + m[0][2];
				n3y = m[1][3] + m[1][2];
				n3z = m[2][3] + m[2][2];
				d3 = m[3][3] + m[3][2]; // near
				break;
			case 3: // left, top, near
				n1x = m[0][3] + m[0][0];
				n1y = m[1][3] + m[1][0];
				n1z = m[2][3] + m[2][0];
				d1 = m[3][3] + m[3][0]; // left
				n2x = m[0][3] - m[0][1];
				n2y = m[1][3] - m[1][1];
				n2z = m[2][3] - m[2][1];
				d2 = m[3][3] - m[3][1]; // top
				n3x = m[0][3] + m[0][2];
				n3y = m[1][3] + m[1][2];
				n3z = m[2][3] + m[2][2];
				d3 = m[3][3] + m[3][2]; // near
				break;
			case 4: // right, bottom, far
				n1x = m[0][3] - m[0][0];
				n1y = m[1][3] - m[1][0];
				n1z = m[2][3] - m[2][0];
				d1 = m[3][3] - m[3][0]; // right
				n2x = m[0][3] + m[0][1];
				n2y = m[1][3] + m[1][1];
				n2z = m[2][3] + m[2][1];
				d2 = m[3][3] + m[3][1]; // bottom
				n3x = m[0][3] - m[0][2];
				n3y = m[1][3] - m[1][2];
				n3z = m[2][3] - m[2][2];
				d3 = m[3][3] - m[3][2]; // far
				break;
			case 5: // left, bottom, far
				n1x = m[0][3] + m[0][0];
				n1y = m[1][3] + m[1][0];
				n1z = m[2][3] + m[2][0];
				d1 = m[3][3] + m[3][0]; // left
				n2x = m[0][3] + m[0][1];
				n2y = m[1][3] + m[1][1];
				n2z = m[2][3] + m[2][1];
				d2 = m[3][3] + m[3][1]; // bottom
				n3x = m[0][3] - m[0][2];
				n3y = m[1][3] - m[1][2];
				n3z = m[2][3] - m[2][2];
				d3 = m[3][3] - m[3][2]; // far
				break;
			case 6: // left, top, far
				n1x = m[0][3] + m[0][0];
				n1y = m[1][3] + m[1][0];
				n1z = m[2][3] + m[2][0];
				d1 = m[3][3] + m[3][0]; // left
				n2x = m[0][3] - m[0][1];
				n2y = m[1][3] - m[1][1];
				n2z = m[2][3] - m[2][1];
				d2 = m[3][3] - m[3][1]; // top
				n3x = m[0][3] - m[0][2];
				n3y = m[1][3] - m[1][2];
				n3z = m[2][3] - m[2][2];
				d3 = m[3][3] - m[3][2]; // far
				break;
			case 7: // right, top, far
				n1x = m[0][3] - m[0][0];
				n1y = m[1][3] - m[1][0];
				n1z = m[2][3] - m[2][0];
				d1 = m[3][3] - m[3][0]; // right
				n2x = m[0][3] - m[0][1];
				n2y = m[1][3] - m[1][1];
				n2z = m[2][3] - m[2][1];
				d2 = m[3][3] - m[3][1]; // top
				n3x = m[0][3] - m[0][2];
				n3y = m[1][3] - m[1][2];
				n3z = m[2][3] - m[2][2];
				d3 = m[3][3] - m[3][2]; // far
				break;
			default:
				throw MILO_RUNTIME_EXCEPTION("Invalid corner value");
		}
		float c23x, c23y, c23z;
		c23x = n2y * n3z - n2z * n3y;
		c23y = n2z * n3x - n2x * n3z;
		c23z = n2x * n3y - n2y * n3x;
		float c31x, c31y, c31z;
		c31x = n3y * n1z - n3z * n1y;
		c31y = n3z * n1x - n3x * n1z;
		c31z = n3x * n1y - n3y * n1x;
		float c12x, c12y, c12z;
		c12x = n1y * n2z - n1z * n2y;
		c12y = n1z * n2x - n1x * n2z;
		c12z = n1x * n2y - n1y * n2x;
		float invDot = 1.0f / (n1x * c23x + n1y * c23y + n1z * c23z);

		Vector3 result;

		result.x = (- c23x * d1 - c31x * d2 - c12x * d3) * invDot;
		result.y = (- c23y * d1 - c31y * d2 - c12y * d3) * invDot;
		result.z = (- c23z * d1 - c31z * d2 - c12z * d3) * invDot;

		return result;
	}
}
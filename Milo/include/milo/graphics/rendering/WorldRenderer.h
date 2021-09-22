#pragma once

#include "FrameGraph.h"
#include "milo/scenes/Scene.h"
#include "milo/graphics/rendering/GraphicsPresenter.h"


namespace milo {

	struct DrawCommand {

		Matrix4 transform{Matrix4(1.0f)};
		Mesh* mesh{nullptr};
		Material* material{nullptr};

		inline uint64_t hash() const noexcept {
			// TODO: use handles
			uint64_t materialId = (uint64_t) material;
			uint32_t meshId = (uint32_t)(((uint64_t) mesh) & 0xFFFFFFFF);
			return (materialId << 32) | meshId;
		}

		inline bool operator<(const DrawCommand& other) const noexcept {
			return hash() < other.hash();
		}
	};

	struct LightEnvironment {
		Skybox* skybox{nullptr};
		Optional<DirectionalLight> dirLight{};
		ArrayList<PointLight> pointLights;
		Color ambientColor{0.2f, 0.2f, 0.2f, 1.0f};
	};

	struct ShadowCascade {
		float splitDepth{0};
		Matrix4 viewProj = Matrix4(1.0f);
		Matrix4 view = Matrix4(1.0f);
	};

	struct CameraInfo {
		Matrix4 proj = Matrix4(1.0);
		Matrix4 view = Matrix4(1.0);
		Matrix4 projView = Matrix4(1.0);
		Polyhedron frustum{};
		Vector3 position{};
		float aspect{0};
	};

	class WorldRenderer {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		GraphicsPresenter* m_GraphicsPresenter = nullptr;
		FrameGraphResourcePool* m_ResourcePool = nullptr;
		FrameGraph m_FrameGraph;
		bool m_ShowGrid{false};
		bool m_ShadowsEnabled{true};
		bool m_ShowBoundingVolumes{false};
		bool m_ShowShadowCascades{false};
		bool m_SoftShadows{false};
		bool m_ShadowCascadeFading{false};
		float m_CascadeFading{1};
		ArrayList<DrawCommand> m_DrawCommands;
		ArrayList<DrawCommand> m_ShadowDrawCommands;
		CameraInfo m_Camera{};
		LightEnvironment m_LightEnvironment{};
		float m_ShadowsMaxDistance{200};
		Size m_ShadowsMapSize{4096, 4096};
		Array<ShadowCascade, 4> m_ShadowCascades{};
	private:
		WorldRenderer();
		~WorldRenderer();
		void render(Scene* scene);
	public:
		FrameGraphResourcePool& resources() const;
		Framebuffer& getFramebuffer() const;
		bool showGrid() const;
		void setShowGrid(bool show);
		bool shadowsEnabled() const;
		void setShadowsEnabled(bool shadowsEnabled);
		bool showBoundingVolumes() const;
		void setShowBoundingVolumes(bool value);
		bool showShadowCascades() const;
		void setShowShadowCascades(bool value);
		bool softShadows() const;
		void setSoftShadows(bool value);
		bool shadowCascadeFading() const;
		void setShadowCascadeFading(bool value);
		bool shadowCascadeFadingValue() const;
		void setShadowCascadeFadingValue(float value);
		void submit(DrawCommand drawCommand, bool castShadows);
		const ArrayList<DrawCommand>& drawCommands() const;
		const ArrayList<DrawCommand>& shadowsDrawCommands() const;
		const CameraInfo& camera() const;
		const LightEnvironment& lights() const;
		float shadowsMaxDistance() const;
		void setShadowsMaxDistance(float distance);
		const Size& shadowsMapSize() const;
		void setShadowsMapSize(const Size& size);
		const Array<ShadowCascade, 4>& shadowCascades() const;
	private:
		static WorldRenderer* s_Instance;
	public:
		static WorldRenderer& get();
	private:
		static void render();
		static void update();
		static void generateDrawCommands(Scene* scene);
		static void init();
		static void shutdown();
		static void getCameraInfo(Scene* scene);
		static void generateLightEnvironment(Scene* scene);
		static void calculateShadowCascades(Scene* scene);
		static void calculateShadowCascadeRanges(float cascadeRanges[5], float zNear, float zFar);
		static Vector3 getFrustumCorner(const Matrix4& projView, uint32_t corner);
	};

}
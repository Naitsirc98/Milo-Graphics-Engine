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
		DirectionalLight dirLight{};
		ArrayList<PointLight> pointLights;
		Color ambientColor{0.2f, 0.2f, 0.2f, 1.0f};
	};

	struct CameraInfo {
		Matrix4 proj = Matrix4(1.0);
		Matrix4 view = Matrix4(1.0);
		Matrix4 projView = Matrix4(1.0);
		Polyhedron frustum{};
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
		ArrayList<DrawCommand> m_DrawCommands;
		ArrayList<DrawCommand> m_ShadowDrawCommands;
		CameraInfo m_Camera{};
		LightEnvironment m_LightEnvironment{};
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
		void submit(DrawCommand drawCommand, bool castShadows);
		const ArrayList<DrawCommand>& drawCommands() const;
		const ArrayList<DrawCommand>& shadowsDrawCommands() const;
		const CameraInfo& camera() const;
		const LightEnvironment& lights() const;
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
	};

}
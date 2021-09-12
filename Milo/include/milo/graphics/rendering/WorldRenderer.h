#pragma once

#include "FrameGraph.h"
#include "milo/scenes/Scene.h"
#include "milo/graphics/rendering/GraphicsPresenter.h"

namespace milo {

	struct DrawCommand {

		Matrix4 transform{Matrix4(1.0f)};
		Mesh* mesh{nullptr};
		Material* material{nullptr};

		inline bool operator<(const DrawCommand& other) const noexcept {
			return ((uint64_t)material) < ((uint64_t)other.material);
		}
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
		ArrayList<DrawCommand> m_DrawCommands;
		ArrayList<DrawCommand> m_ShadowDrawCommands;
	private:
		WorldRenderer();
		~WorldRenderer();
		void render(Scene* scene);
	public:
		Framebuffer& getFramebuffer() const;
		bool showGrid() const;
		void setShowGrid(bool show);
		bool shadowsEnabled() const;
		void setShadowsEnabled(bool shadowsEnabled);
		void submit(DrawCommand drawCommand, bool castShadows);
		const ArrayList<DrawCommand>& drawCommands() const;
		const ArrayList<DrawCommand>& shadowsDrawCommands() const;
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
	};

}
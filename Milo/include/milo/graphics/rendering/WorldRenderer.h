#pragma once

#include "FrameGraph.h"
#include "milo/scenes/Scene.h"
#include "milo/graphics/rendering/GraphicsPresenter.h"

namespace milo {

	class WorldRenderer {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		GraphicsPresenter* m_GraphicsPresenter = nullptr;
		FrameGraphResourcePool* m_ResourcePool = nullptr;
		FrameGraph m_FrameGraph;
	private:
		WorldRenderer();
		~WorldRenderer();
		void render(Scene* scene);
	private:
		static WorldRenderer* s_Instance;
	private:
		static void render();
		static void init();
		static void shutdown();
	};

}
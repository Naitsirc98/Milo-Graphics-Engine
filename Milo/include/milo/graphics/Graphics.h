#pragma once

#include "GraphicsContext.h"
#include "GraphicsAPI.h"
#include "Window.h"

namespace milo {

	struct Vertex3D {
		Vector3 position = {0, 0, 0};
		Vector3 normal = {0, 0, 0};
		Vector2 texCoords = {0, 0};
	};

	class Graphics {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static GraphicsAPI s_GraphicsAPI;
		static GraphicsContext* s_GraphicsContext;
	public:
		static GraphicsAPI graphicsAPI();
		static GraphicsContext& graphicsContext();
	private:
		static void init();
		static void shutdown();
	};
}
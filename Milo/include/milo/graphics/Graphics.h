#pragma once

#include "GraphicsContext.h"
#include "GraphicsAPI.h"
#include "Window.h"
#include "Vertex.h"
#include "milo/time/Profiler.h"

namespace milo {

	class Graphics {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static GraphicsAPI s_GraphicsAPI;
		static GraphicsContext* s_GraphicsContext;
	public:
		static GraphicsAPI graphicsAPI();
		static GraphicsContext* graphicsContext();
	private:
		static void init();
		static void shutdown();
	};
}
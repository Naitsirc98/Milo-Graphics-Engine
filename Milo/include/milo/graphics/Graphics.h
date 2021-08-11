#pragma once

#include <milo/graphics/api/GraphicsContext.h>
#include "GraphicsAPI.h"

namespace milo {

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
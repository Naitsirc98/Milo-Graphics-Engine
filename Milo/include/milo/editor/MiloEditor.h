#pragma once

#include "milo/common/Common.h"
#include "UIRenderer.h"

namespace milo {

	class MiloEditor {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		MiloEditor() = default;
		~MiloEditor() = default;
	private:
		static UIRenderer* s_Renderer;
	public:
		static void render();
	private:
		static void init();
		static void shutdown();
	};
}
#pragma once

#include "milo/common/Common.h"
#include "UIRenderer.h"
#include "SceneHierarchyPanel.h"

namespace milo {

	class MiloEditor {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		MiloEditor() = default;
		~MiloEditor() = default;
	private:
		static UIRenderer* s_Renderer;
		static SceneHierarchyPanel s_SceneHierarchyPanel;
	public:
		static void render();
	private:
		static void setupDockSpace();
		static void init();
		static void shutdown();
	};
}
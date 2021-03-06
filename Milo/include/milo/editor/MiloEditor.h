#pragma once

#include "milo/common/Common.h"
#include "UIRenderer.h"
#include "SceneHierarchyPanel.h"
#include "PropertiesPanel.h"
#include "EditorCamera.h"
#include "MaterialEditor.h"
#include "DockSpaceRenderer.h"

namespace milo {

	class MiloEditor {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		MiloEditor() = default;
		~MiloEditor() = default;
	private:
		static UIRenderer* s_Renderer;
		static EditorCamera s_Camera;
		static SceneHierarchyPanel s_SceneHierarchyPanel;
		static PropertiesPanel s_PropertiesPanel;
		static MaterialEditor s_MaterialEditor;
		static DockSpaceRenderer s_DockSpaceRenderer;

		static Material* s_MaterialToEdit;
	public:
		static void update();
		static void render();
		static EditorCamera& camera();
	private:
		static void renderSceneViewport();
		static void setupDockSpace();
		static void init();
		static void shutdown();
		static void setupMenuBar();

		static void setupDockSpace(const char* dockSpaceWindowName, const char* dockSpaceName, bool& firstTime);
	};
}
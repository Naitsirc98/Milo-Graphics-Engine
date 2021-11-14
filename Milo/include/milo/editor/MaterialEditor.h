#pragma once

#include "UIRenderer.h"
#include "milo/assets/AssetManager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include "milo/editor/blueprints/Blueprints.h"
#include "milo/editor/DockSpaceRenderer.h"
#include "MaterialViewerRenderer.h"

namespace ed = ax::NodeEditor;

namespace milo {

	struct LinkInfo {
		ed::LinkId id;
		ed::PinId inputId;
		ed::PinId outputId;
	};

	class MaterialEditor {
	private:
		Material* m_LastMaterial{nullptr};
		bool m_Open = false;
		ed::EditorContext* m_Context{nullptr};
		bool m_FirstFrame{true};
		ImVector<LinkInfo> m_Links;
		int32_t m_NextId{100};
		bp::BlueprintBuilder m_BlueprintBuilder;
		DockSpaceRenderer m_DockSpaceRenderer;
		MaterialViewerRenderer* m_MaterialViewerRenderer;
	public:
		MaterialEditor();
		~MaterialEditor();
		void render(Material* material);
		bool isOpen() const;
		void setOpen(bool open);
	private:
		void beginColumn();
		void nextColumn();
		void endColumn();
		void createIcon(bp::Icon& icon, const String& textureFile);
		void renderMaterialViewer(Material* material);
		int32_t getNextId();
		void setMaterialData(Material* material);
		void fetchMaterialDataFromNodes();
	};
}
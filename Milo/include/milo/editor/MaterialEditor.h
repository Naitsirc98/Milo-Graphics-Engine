#pragma once

#include "UIRenderer.h"
#include "milo/assets/AssetManager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include "milo/editor/blueprints/Blueprints.h"
#include "milo/editor/DockSpaceRenderer.h"

namespace ed = ax::NodeEditor;

namespace milo {

	struct LinkInfo {
		ed::LinkId id;
		ed::PinId inputId;
		ed::PinId outputId;
	};

	class MaterialEditor {
	private:
		bool m_Open = false;
		ed::EditorContext* m_Context{nullptr};
		bool m_FirstFrame{true};
		ImVector<LinkInfo> m_Links;
		int32_t m_NextLinkId{100};
		bp::BlueprintBuilder m_BlueprintBuilder;
		DockSpaceRenderer m_DockSpaceRenderer;
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
	};

}
#pragma once

#include "UIRenderer.h"
#include "milo/assets/AssetManager.h"
#include <imgui_node_editor.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

namespace ed = ax::NodeEditor;

namespace milo {

	struct LinkInfo {
		ed::LinkId id;
		ed::PinId inputId;
		ed::PinId outputId;
	};

	class MaterialEditor {
	private:
		ed::EditorContext* m_Context{nullptr};
		bool m_FirstFrame{true};
		ImVector<LinkInfo> m_Links;
		int32_t m_NextLinkId{100};
	public:
		MaterialEditor();
		~MaterialEditor();
		void render(Material* material);
	private:
		void beginColumn();
		void nextColumn();
		void endColumn();
	};

}
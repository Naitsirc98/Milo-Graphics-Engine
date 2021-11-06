#pragma once

#include "UIRenderer.h"
#include "milo/assets/AssetManager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include "milo/editor/blueprints/Blueprints.h"

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
		bp::BlueprintBuilder m_BlueprintBuilder;
	public:
		MaterialEditor();
		~MaterialEditor();
		void render(Material* material);
	private:
		void beginColumn();
		void nextColumn();
		void endColumn();
		void createIcon(bp::Icon& icon, const String& textureFile);
	};

}
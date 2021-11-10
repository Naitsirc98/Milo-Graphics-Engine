#pragma once

#include "milo/common/Common.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace milo {

	class DockSpaceRenderer {
	private:
		bool m_FirstFrame{true};
		String m_DockSpaceWindowName;
		String m_DockSpaceName;
		Function<void, ImGuiID> m_OnFirstFrame;
		Function<void> m_OnRender;
	public:
		DockSpaceRenderer() = default;
		DockSpaceRenderer(String dockSpaceWindowName, String dockSpaceName, Function<void, ImGuiID> onFirstFrame, Function<void> onRender);
		void render();
		void setFirstFrame(bool firstFrame);
		void setDockSpaceWindowName(const String& mDockSpaceWindowName);
		void setDockSpaceName(const String& mDockSpaceName);
		void setOnFirstFrame(const Function<void, ImGuiID>& mOnFirstFrame);
		void setOnRender(const Function<void>& mOnRender);
	};
}
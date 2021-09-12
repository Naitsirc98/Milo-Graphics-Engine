#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/textures/Texture.h"
#include <imgui/imgui.h>

namespace milo {

	class UIRenderer {
		friend class MiloEditor;
	protected:
		UIRenderer() = default;
		virtual ~UIRenderer() = default;
	public:
		virtual void begin() = 0;
		virtual void end() = 0;
	};

	namespace UI {

		static bool isMouseEnabled() {
			return ImGui::GetIO().ConfigFlags & ~ImGuiConfigFlags_NoMouse;
		}

		static void setMouseEnabled(bool enable) {
			if (enable)
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			else
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		void image(const Texture2D& texture, const Size& size = {0, 0}, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1),
				   const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));

		void imageButton(const String& title, const Texture2D& texture, const Size& size = {0, 0});

		void beginGrid(uint32_t columns = 2);

		void endGrid();

		namespace FileDialog {

			Optional<String> open(const char* filter);

			Optional<String> save(const char* filter);
		}
	}

}
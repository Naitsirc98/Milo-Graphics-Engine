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
	public:
		// TODO
	};

	namespace UI {

		void image(const Texture2D& texture, const Size& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1,1),
				   const ImVec4& tint_col = ImVec4(1,1,1,1), const ImVec4& border_col = ImVec4(0,0,0,0));
	}

}
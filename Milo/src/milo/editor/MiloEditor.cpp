#include "milo/editor/MiloEditor.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/assets/AssetManager.h"
#include <imgui/imgui.h>

namespace milo {

	UIRenderer* MiloEditor::s_Renderer = nullptr;

	void MiloEditor::render() {

		s_Renderer->begin();

		bool open = true;
		ImGui::ShowDemoWindow(&open);

		const Texture2D* texture = WorldRenderer::get().getRenderTarget().colorAttachment;

		ImVec2 size;
		size.x = (float)texture->width();
		size.y = (float)texture->height();

		UI::image(*texture, size);

		s_Renderer->end();
	}

	void MiloEditor::init() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			s_Renderer = new VulkanUIRenderer();
		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}
	}

	void MiloEditor::shutdown() {
		DELETE_PTR(s_Renderer);
	}

}
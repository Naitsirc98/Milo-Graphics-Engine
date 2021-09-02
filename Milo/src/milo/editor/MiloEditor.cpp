#include "milo/editor/MiloEditor.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"

#include <imgui.h>

namespace milo {

	UIRenderer* MiloEditor::s_Renderer = nullptr;

	void MiloEditor::render() {

		s_Renderer->begin();

		bool open = true;

		ImGui::SetNextWindowSize({300, 300});

		ImGui::Begin("Hola", &open);

		ImGui::Text("Hello World!!");

		ImGui::End();

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
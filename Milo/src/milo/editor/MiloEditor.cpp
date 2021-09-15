#include "milo/editor/MiloEditor.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/assets/AssetManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace milo {

	UIRenderer* MiloEditor::s_Renderer = nullptr;
	EditorCamera MiloEditor::s_Camera{};
	SceneHierarchyPanel MiloEditor::s_SceneHierarchyPanel{};
	PropertiesPanel MiloEditor::s_PropertiesPanel{};

	void MiloEditor::update() {
		s_Camera.update();
	}

	void MiloEditor::render() {

		s_Renderer->begin();

		setupDockSpace();

		s_SceneHierarchyPanel.render();

		if(s_SceneHierarchyPanel.selectedEntity()) {
			s_PropertiesPanel.render(s_SceneHierarchyPanel.selectedEntity());
		} else {
			ImGui::Begin("PropertiesPanel");
			ImGui::End();
		}

		renderSceneViewport();

		s_Renderer->end();
	}

	void MiloEditor::renderSceneViewport() {
		ImGui::Begin("SceneViewportPanel", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		auto* texture = WorldRenderer::get().getFramebuffer().colorAttachments()[0];
		UI::image(*texture, texture->size());
		SceneManager::activeScene()->setFocused(ImGui::IsWindowFocused());
		ImGui::End();
	}

	void MiloEditor::setupDockSpace() {
		// Based on https://gist.github.com/PossiblyAShrub/0aea9511b84c34e191eaa90dd7225969

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		setupMenuBar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {

			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

			static bool first_time = true;
			if (first_time) {
				first_time = false;

				ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
				ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

				ImGuiID left = NULL;
				ImGuiID upperLeft = NULL;
				ImGuiID lowerLeft = NULL;

				ImGuiID right = NULL;
				ImGuiID upperRight = NULL;
				ImGuiID lowerRight = NULL;

				// Split screen into left and right sections
				ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, &left, &right);
				// Split left side by 2: upper (scene hierarchy) and lower (properties)
				ImGui::DockBuilderSplitNode(left, ImGuiDir_Up, 0.50f, &upperLeft, &lowerLeft);
				// Split right side by 2: upper (scene viewport) and lower (asset explorer)
				//ImGui::DockBuilderSplitNode(right, ImGuiDir_Up, 0.70f, &upperRight, &lowerRight);

				ImGui::DockBuilderDockWindow("SceneHierarchyPanel", upperLeft);
				ImGui::DockBuilderDockWindow("PropertiesPanel", lowerLeft);
				ImGui::DockBuilderDockWindow("SceneViewportPanel", right);
				//ImGui::DockBuilderDockWindow("ContentBrowserPanel", lowerRight);

				ImGui::DockBuilderFinish(dockspace_id);
			}
		}

		ImGui::End();
	}

	void MiloEditor::setupMenuBar() {
		if(ImGui::BeginMainMenuBar()) {

			if(ImGui::BeginMenu("File")) {
				ImGui::MenuItem("New");
				ImGui::MenuItem("Open");
				ImGui::MenuItem("Save");
				ImGui::MenuItem("Save as");

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	EditorCamera &MiloEditor::camera() {
		return s_Camera;
	}

	void MiloEditor::init() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			s_Renderer = new VulkanUIRenderer();
		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}

		Size size = Window::get()->size();

		s_Camera = EditorCamera(perspectiveFov(radians(45.0f), (float)size.width, (float)size.height, 0.05f, 100000.0f));
	}

	void MiloEditor::shutdown() {
		DELETE_PTR(s_Renderer);
	}

}
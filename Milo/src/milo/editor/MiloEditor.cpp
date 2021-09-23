#include "milo/editor/MiloEditor.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/ui/VulkanUIRenderer.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/assets/AssetManager.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_node_editor.h>
#include "milo/time/Profiler.h"

namespace milo {

	UIRenderer* MiloEditor::s_Renderer = nullptr;
	EditorCamera MiloEditor::s_Camera{};
	SceneHierarchyPanel MiloEditor::s_SceneHierarchyPanel{};
	PropertiesPanel MiloEditor::s_PropertiesPanel{};
	MaterialEditor MiloEditor::s_MaterialEditor{};

	void MiloEditor::update() {
		MILO_PROFILE_FUNCTION;
		s_Camera.update();
	}

	void MiloEditor::render() {

		MILO_PROFILE_FUNCTION;

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

		//if(true) {
		//	s_MaterialEditor.render(Assets::materials().getDefault());
		//}

		s_Renderer->end();
	}

	void MiloEditor::renderSceneViewport() {
		ImGui::Begin("SceneViewportPanel", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		Texture2D* texture = nullptr;
		if(false) {
			texture = WorldRenderer::get().resources().getFramebuffer(PreDepthRenderPass::getFramebufferHandle())->colorAttachments()[0];
		} else {
			texture = WorldRenderer::get().getFramebuffer().colorAttachments()[0];
		}
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

			setupMenuBar();
		}

		ImGui::End();
	}

	void MiloEditor::setupMenuBar() {

		if(ImGui::BeginMainMenuBar()) {

			static bool depthBufferOpened = false;
			static bool settingsPanelOpened = false;

			if(ImGui::BeginMenu("Options")) {

				if(ImGui::MenuItem("Show Pre-Depth Buffer")) {
					depthBufferOpened = true;
				}

				if(ImGui::MenuItem("More...")) {
					settingsPanelOpened = true;
				}

				ImGui::EndMenu();
			}

			if(depthBufferOpened) {
				if(ImGui::Begin("Pre-Depth buffer", &depthBufferOpened)) {
					auto* texture = WorldRenderer::get().resources().getFramebuffer(
							PreDepthRenderPass::getFramebufferHandle())->colorAttachments()[1];
					UI::image(*texture, texture->size());
					ImGui::End();
				}
			}

			if(settingsPanelOpened) {
				if(ImGui::Begin("Settings", &settingsPanelOpened)) {

					bool shadowsEnabled = WorldRenderer::get().shadowsEnabled();
					bool showBoundingVolumes = WorldRenderer::get().showBoundingVolumes();
					bool showGrid = WorldRenderer::get().showGrid();
					bool showShadowCascades = WorldRenderer::get().showShadowCascades();
					bool softshadows = WorldRenderer::get().softShadows();
					bool cascadeFadingEnabled = WorldRenderer::get().shadowCascadeFading();
					bool cascadeFadingValue = WorldRenderer::get().shadowCascadeFadingValue();

					float shadowsMaxDistance = WorldRenderer::get().shadowsMaxDistance();

					uint32_t drawCommandsCount = WorldRenderer::get().drawCommands().size();

					ImGui::Checkbox("Shadows enabled", &shadowsEnabled);
					ImGui::Checkbox("Show shadow cascades", &showShadowCascades);
					ImGui::Checkbox("Show bounding volumes", &showBoundingVolumes);
					ImGui::Checkbox("Show grid", &showGrid);
					ImGui::Checkbox("Soft Shadows", &softshadows);
					ImGui::Checkbox("Cascade fading enabled", &cascadeFadingEnabled);
					ImGui::Checkbox("Cascade fading value", &cascadeFadingValue);

					ImGui::DragFloat("Shadows max distance", &shadowsMaxDistance);

					ImGui::Text("Draw commands count: %i", drawCommandsCount);

					WorldRenderer::get().setShadowsEnabled(shadowsEnabled);
					WorldRenderer::get().setShowBoundingVolumes(showBoundingVolumes);
					WorldRenderer::get().setShowGrid(showGrid);
					WorldRenderer::get().setShowShadowCascades(showShadowCascades);
					WorldRenderer::get().setSoftShadows(softshadows);
					WorldRenderer::get().setShadowCascadeFading(cascadeFadingEnabled);
					WorldRenderer::get().setShadowCascadeFadingValue(cascadeFadingValue);

					WorldRenderer::get().setShadowsMaxDistance(shadowsMaxDistance);

					ImGui::End();
				}
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

		s_Camera = EditorCamera(perspectiveFov(radians(45.0f), (float)size.width, (float)size.height, 0.1f, 1000.0f));
		//s_Camera.m_Position = {0, 0, 0};
	}

	void MiloEditor::shutdown() {
		DELETE_PTR(s_Renderer);
	}

}
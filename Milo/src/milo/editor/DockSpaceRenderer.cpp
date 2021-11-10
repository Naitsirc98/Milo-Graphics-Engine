#include <utility>

#include "milo/editor/DockSpaceRenderer.h"

namespace milo {

	DockSpaceRenderer::DockSpaceRenderer(String dockSpaceWindowName, String dockSpaceName,
										 Function<void, ImGuiID> onFirstFrame, Function<void> onRender)
									 : m_DockSpaceWindowName(std::move(dockSpaceWindowName)), m_DockSpaceName(std::move(dockSpaceName)),
									   m_OnFirstFrame(std::move(onFirstFrame)), m_OnRender(std::move(onRender)) {
	}

	void DockSpaceRenderer::render() {
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
		ImGui::Begin(m_DockSpaceWindowName.c_str(), nullptr, window_flags);
		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {

			ImGuiID dockSpaceId = ImGui::GetID(m_DockSpaceName.c_str());
			ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockspace_flags);

			if (m_FirstFrame) {
				m_FirstFrame = false;

				ImGui::DockBuilderRemoveNode(dockSpaceId); // clear any previous layout
				ImGui::DockBuilderAddNode(dockSpaceId, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(dockSpaceId, viewport->Size);

				m_OnFirstFrame(dockSpaceId);
			}

			m_OnRender();
		}

		ImGui::End();
	}

	void DockSpaceRenderer::setFirstFrame(bool firstFrame) {
		m_FirstFrame = firstFrame;
	}

	void DockSpaceRenderer::setDockSpaceWindowName(const String& mDockSpaceWindowName) {
		m_DockSpaceWindowName = mDockSpaceWindowName;
	}

	void DockSpaceRenderer::setDockSpaceName(const String& mDockSpaceName) {
		m_DockSpaceName = mDockSpaceName;
	}

	void DockSpaceRenderer::setOnFirstFrame(const Function<void, ImGuiID>& mOnFirstFrame) {
		m_OnFirstFrame = mOnFirstFrame;
	}

	void DockSpaceRenderer::setOnRender(const Function<void>& mOnRender) {
		m_OnRender = mOnRender;
	}
}
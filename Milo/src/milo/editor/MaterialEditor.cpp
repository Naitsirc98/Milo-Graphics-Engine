#include "milo/editor/MaterialEditor.h"

namespace milo {

	MaterialEditor::MaterialEditor() {
		m_Context = ed::CreateEditor();
	}

	MaterialEditor::~MaterialEditor() {
		ed::DestroyEditor(m_Context);
	}

	void MaterialEditor::render(Material* material) {
		if(material == nullptr) return;

		auto& io = ImGui::GetIO();

		ImGui::Text("Material: %s", material->name().c_str());

		// TODO: render this material

		ImGui::Separator();

		ed::SetCurrentEditor(m_Context);

		// Start interaction with editor.
		String s = str("Edit ") + material->name();
		ed::Begin(s.c_str(), ImVec2(0.0, 0.0f));

		int uniqueId = 1;

		//
		// 1) Commit known data to editor
		//

		// Submit Node A
		ed::NodeId nodeA_Id = uniqueId++;
		ed::PinId  nodeA_InputPinId = uniqueId++;
		ed::PinId  nodeA_OutputPinId = uniqueId++;

		if (m_FirstFrame)
			ed::SetNodePosition(nodeA_Id, ImVec2(10, 10));
		ed::BeginNode(nodeA_Id);
		ImGui::Text("Node A");
		ed::BeginPin(nodeA_InputPinId, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(nodeA_OutputPinId, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();

		// Submit Node B
		ed::NodeId nodeB_Id = uniqueId++;
		ed::PinId  nodeB_InputPinId1 = uniqueId++;
		ed::PinId  nodeB_InputPinId2 = uniqueId++;
		ed::PinId  nodeB_OutputPinId = uniqueId++;

		if (m_FirstFrame)
			ed::SetNodePosition(nodeB_Id, ImVec2(210, 60));
		ed::BeginNode(nodeB_Id);
		ImGui::Text("Node B");
		beginColumn();
		ed::BeginPin(nodeB_InputPinId1, ed::PinKind::Input);
		ImGui::Text("-> In1");
		ed::EndPin();
		ed::BeginPin(nodeB_InputPinId2, ed::PinKind::Input);
		ImGui::Text("-> In2");
		ed::EndPin();
		nextColumn();
		ed::BeginPin(nodeB_OutputPinId, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		endColumn();
		ed::EndNode();

		// Submit Links
		for (auto& linkInfo : m_Links)
			ed::Link(linkInfo.id, linkInfo.inputId, linkInfo.outputId);

		//
		// 2) Handle interactions
		//

		// Handle creation action, returns true if editor want to create new object (node or link)
		if (ed::BeginCreate())
		{
			ed::PinId inputPinId, outputPinId;
			if (ed::QueryNewLink(&inputPinId, &outputPinId))
			{
				// QueryNewLink returns true if editor want to create new link between pins.
				//
				// Link can be created only for two valid pins, it is up to you to
				// validate if connection make sense. Editor is happy to make any.
				//
				// Link always goes from input to output. User may choose to drag
				// link from output pin or input pin. This determine which pin ids
				// are valid and which are not:
				//   * input valid, output invalid - user started to drag new ling from input pin
				//   * input invalid, output valid - user started to drag new ling from output pin
				//   * input valid, output valid   - user dragged link over other pin, can be validated

				if (inputPinId && outputPinId) // both are valid, let's accept link
				{
					// ed::AcceptNewItem() return true when user release mouse button.
					if (ed::AcceptNewItem())
					{
						// Since we accepted new link, lets add one to our list of links.
						m_Links.push_back({ ed::LinkId(m_NextLinkId++), inputPinId, outputPinId });

						// Draw new link.
						ed::Link(m_Links.back().id, m_Links.back().inputId, m_Links.back().outputId);
					}

					// You may choose to reject connection between these nodes
					// by calling ed::RejectNewItem(). This will allow editor to give
					// visual feedback by changing link thickness and color.
				}
			}
		}
		ed::EndCreate(); // Wraps up object creation action handling.


		// Handle deletion action
		if (ed::BeginDelete())
		{
			// There may be many links marked for deletion, let's loop over them.
			ed::LinkId deletedLinkId;
			while (ed::QueryDeletedLink(&deletedLinkId))
			{
				// If you agree that link can be deleted, accept deletion.
				if (ed::AcceptDeletedItem())
				{
					// Then remove link from your data.
					for (auto& link : m_Links)
					{
						if (link.id == deletedLinkId)
						{
							m_Links.erase(&link);
							break;
						}
					}
				}

				// You may reject link deletion by calling:
				// ed::RejectDeletedItem();
			}
		}
		ed::EndDelete(); // Wrap up deletion action

		// End of interaction with editor.
		ed::End();

		if (m_FirstFrame)
			ed::NavigateToContent(0.0f);

		ed::SetCurrentEditor(nullptr);

		m_FirstFrame = false;
	}

	void MaterialEditor::beginColumn() {
		ImGui::BeginGroup();
	}

	void MaterialEditor::nextColumn() {
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
	}

	void MaterialEditor::endColumn() {
		ImGui::EndGroup();
	}

}
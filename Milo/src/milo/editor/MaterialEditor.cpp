#include "milo/editor/MaterialEditor.h"

namespace milo {

	MaterialEditor::MaterialEditor() {

		ed::Config config;

		config.UserPointer = &m_BlueprintBuilder;

		config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t {

			bp::BlueprintBuilder* bb = (bp::BlueprintBuilder*) userPointer;

			auto node = bb->findNode(nodeId);
			if (!node) return 0;

			if (data != nullptr) memcpy(data, node->state.data(), node->state.size());

			return node->state.size();
		};

		config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool {

			bp::BlueprintBuilder* bb = (bp::BlueprintBuilder*) userPointer;

			auto node = bb->findNode(nodeId);
			if (!node) return false;

			node->state.assign(data, size);

			bb->touchNode(nodeId);

			return true;
		};

		m_Context = ed::CreateEditor(&config);
		//ed::SetCurrentEditor(m_Context);

		//createIcon(m_BlueprintBuilder.getHeaderBackground(), "");
		//createIcon(m_BlueprintBuilder.getSaveIcon(), "");
		//createIcon(m_BlueprintBuilder.getRestoreIcon(), "");

		m_DockSpaceRenderer.setDockSpaceWindowName("MaterialEditorDockSpaceWindow");
		m_DockSpaceRenderer.setDockSpaceName("MaterialEditorDockSpace");
		m_DockSpaceRenderer.setOnFirstFrame([&](ImGuiID dockSpaceId) {

			ImGuiID left = NULL;
			ImGuiID right = NULL;

			// Split screen into left and right sections
			ImGui::DockBuilderSplitNode(dockSpaceId, ImGuiDir_Left, 0.25f, &left, &right);

			ImGui::DockBuilderDockWindow("MaterialViewer", left);
			ImGui::DockBuilderDockWindow("MaterialNodeEditor", right);

			ImGui::DockBuilderFinish(dockSpaceId);
		});
	}

	MaterialEditor::~MaterialEditor() {
		ed::SetCurrentEditor(nullptr);
		ed::DestroyEditor(m_Context);
	}

	bool MaterialEditor::isOpen() const {
		return m_Open;
	}

	void MaterialEditor::setOpen(bool open) {
		m_Open = open;
	}

	void MaterialEditor::render(Material* material) {
		if(material == nullptr) return;

		auto& io = ImGui::GetIO();

		bool openThisFrame = m_Open;

		if(openThisFrame && ImGui::Begin("Material Editor", &m_Open)) {

			renderMaterialViewer(material);

			ImGui::Separator();

			ed::SetCurrentEditor(m_Context);

			// Start interaction with editor.
			ed::Begin("MaterialNodeEditor", ImVec2(0.0f, 0.0f));

			int uniqueId = 1;

			//
			// 1) Commit known data to editor
			//

			bp::Node nodeA(uniqueId++, "Node A");
			nodeA.id = uniqueId++;
			nodeA.inputs.push_back(bp::Pin(uniqueId++, "Input 1", bp::PinType::Float));
			nodeA.outputs.push_back(bp::Pin(uniqueId++, "Output 1", bp::PinType::Flow));

			// Submit Node A
			{
				if (m_FirstFrame) ed::SetNodePosition(nodeA.id, ImVec2(0, 0));
				ed::BeginNode(nodeA.id);
				ImGui::Text("Node A");
				ed::BeginPin(nodeA.inputs[0].id, ed::PinKind::Input);
				m_BlueprintBuilder.drawPinIcon(nodeA.inputs[0], true, 255);
				ImGui::Text("Float");
				ed::EndPin();
				ImGui::SameLine();
				ed::BeginPin(nodeA.outputs[0].id, ed::PinKind::Output);
				m_BlueprintBuilder.drawPinIcon(nodeA.outputs[0], true, 255);
				//ImGui::Text("Out ->");
				ed::EndPin();
				ed::EndNode();
			}

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

			if(m_FirstFrame) {
				ed::NavigateToContent();
			}

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

		if(openThisFrame) ImGui::End();
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

	void MaterialEditor::createIcon(bp::Icon& icon, const String& textureFile) {

		Image* image = Image::loadImage(textureFile, PixelFormat::RGBA8);

		Ref<Texture2D> texture = Ref<Texture2D>(Texture2D::create(TEXTURE_USAGE_SAMPLED_BIT | TEXTURE_USAGE_UI_BIT));

		Texture2D::AllocInfo allocInfo{};
		allocInfo.format = image->format();
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.pixels = image->pixels();

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		icon.texture = texture;
		icon.iconId = UI::getIconId(*texture);
	}

	void MaterialEditor::renderMaterialViewer(Material* material) {

		ImGui::Begin("MaterialViewer");

		//TODO: real time render of the material
		auto icon = Assets::textures().getIcon("MaterialViewerIcon");
		if(icon == nullptr) {
			icon = Assets::textures().createIcon("MaterialViewerIcon", Assets::meshes().getSphere(), material);
		}
		UI::image(*icon);

		ImGui::Separator();
		ImGui::Text("Name: %s", material->name().c_str());

		ImGui::Separator();
		ImGui::Text("File: %s", material->filename().c_str());

		ImGui::End();

		if(material->dirty()) { // TODO: recreate icon when material is modified
			Assets::textures().removeIcon("MaterialViewerIcon");
			Assets::textures().createIcon("MaterialViewerIcon", Assets::meshes().getSphere(), material);
		}
	}

}
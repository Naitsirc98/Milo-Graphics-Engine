#include "milo/editor/blueprints/Blueprints.h"
#include "milo/editor/UIRenderer.h"
#include "milo/assets/AssetManager.h"

namespace milo::bp {

	BlueprintBuilder::BlueprintBuilder() {
		m_Nodes.reserve(1024);
		m_Links.reserve(1024);
	}

	BlueprintBuilder::~BlueprintBuilder() {

	}

	int32_t BlueprintBuilder::getNextId() {
		return m_NextId++;
	}

	ed::LinkId BlueprintBuilder::getNextLinkId() {
		return ed::LinkId(getNextId());
	}

	void BlueprintBuilder::touchNode(ed::NodeId id) {
		m_NodeTouchTime[id] = m_TouchTime;
	}

	float BlueprintBuilder::getTouchProgress(ed::NodeId id) {
		auto it = m_NodeTouchTime.find(id);
		if (it != m_NodeTouchTime.end() && it->second > 0.0f)
			return (m_TouchTime - it->second) / m_TouchTime;
		else
			return 0.0f;
	}

	void BlueprintBuilder::updateTouch() {
		const auto deltaTime = ImGui::GetIO().DeltaTime;
		for (auto& entry : m_NodeTouchTime) {
			if (entry.second > 0.0f)
				entry.second -= deltaTime;
		}
	}

	Node* BlueprintBuilder::findNode(ed::NodeId id) {
		for (auto& node : m_Nodes)
			if (node.id == id)
				return &node;

		return nullptr;
	}

	Link* BlueprintBuilder::findLink(ed::LinkId id) {
		for (auto& link : m_Links)
			if (link.id == id)
				return &link;

		return nullptr;
	}

	Pin* BlueprintBuilder::findPin(ed::PinId id) {
		if (!id) return nullptr;

		for (auto& node : m_Nodes) {
			for (auto& pin : node.inputs)
				if (pin.id == id)
					return &pin;

			for (auto& pin : node.outputs)
				if (pin.id == id)
					return &pin;
		}

		return nullptr;
	}

	bool BlueprintBuilder::isPinLinked(ed::PinId id) {
		if (!id) return false;

		for (auto& link : m_Links)
			if (link.startPinID == id || link.endPinID == id)
				return true;

		return false;
	}

	bool BlueprintBuilder::canCreateLink(Pin* a, Pin* b) {
		if (!a || !b || a == b || a->kind == b->kind || a->type != b->type || a->node == b->node)
			return false;

		return true;
	}

	const ArrayList<Link>& BlueprintBuilder::links() const {
		return m_Links;
	}

	const ArrayList<Node>& BlueprintBuilder::nodes() const {
		return m_Nodes;
	}

	void BlueprintBuilder::addNode(Node node) {
		m_Nodes.push_back(node);
		buildNode(&m_Nodes.back());
	}

	void BlueprintBuilder::buildNode(Node* node) {
		for (auto& input : node->inputs) {
			input.node = node;
			input.kind = PinKind::Input;
		}

		for (auto& output : node->outputs) {
			output.node = node;
			output.kind = PinKind::Output;
		}
	}

	static inline ImRect ImGui_GetItemRect() {
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y) {
		auto result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	static ImVec2 operator+(const ImVec2& v1, const ImVec2& v2) {
		return ImVec2(v1.x + v1.x, v1.y + v2.y);
	}

	static ImVec2 operator-(const ImVec2& v1, const ImVec2& v2) {
		return ImVec2(v1.x - v1.x, v1.y - v2.y);
	}
	void BlueprintBuilder::draw() {


		static ed::NodeId contextNodeId = 0;
		static ed::LinkId contextLinkId = 0;
		static ed::PinId  contextPinId  = 0;
		static bool createNewNode = false;
		static Pin* newNodeLinkPin = nullptr;
		static Pin* newLinkPin = nullptr;

		ed::Begin("Node editor");
		{
			auto cursorTopLeft = ImGui::GetCursorScreenPos();

			util::BlueprintNodeBuilder builder(m_HeaderBackground.iconId,
											   m_HeaderBackground.texture->size().width,
											   m_HeaderBackground.texture->size().height);

			for (Node& node : m_Nodes) {

				if (node.type != NodeType::Blueprint && node.type != NodeType::Simple)
					continue;

				const auto isSimple = node.type == NodeType::Simple;

				bool hasOutputDelegates = false;
				for (auto& output : node.outputs)
					if (output.type == PinType::Delegate)
						hasOutputDelegates = true;

				builder.Begin(node.id);
				if (!isSimple) {

					builder.Header(node.color);
					ImGui::Spring(0);
					ImGui::TextUnformatted(node.name.c_str());
					ImGui::Spring(1);
					ImGui::Dummy(ImVec2(0, 28));
					if (hasOutputDelegates) {
						ImGui::BeginVertical("delegates", ImVec2(0, 28));
						ImGui::Spring(1, 0);
						for (auto& output : node.outputs)
						{
							if (output.type != PinType::Delegate)
								continue;

							auto alpha = ImGui::GetStyle().Alpha;
							if (newLinkPin && !canCreateLink(newLinkPin, &output) && &output != newLinkPin)
								alpha = alpha * (48.0f / 255.0f);

							ed::BeginPin(output.id, ed::PinKind::Output);
							ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
							ed::PinPivotSize(ImVec2(0, 0));
							ImGui::BeginHorizontal(output.id.AsPointer());
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
							if (!output.name.empty())
							{
								ImGui::TextUnformatted(output.name.c_str());
								ImGui::Spring(0);
							}
							drawPinIcon(output, isPinLinked(output.id), (int)(alpha * 255));
							ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
							ImGui::EndHorizontal();
							ImGui::PopStyleVar();
							ed::EndPin();

							//DrawItemRect(ImColor(255, 0, 0));
						}
						ImGui::Spring(1, 0);
						ImGui::EndVertical();
						ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
					}
					else
						ImGui::Spring(0);
					builder.EndHeader();
				}

				node.drawContent();

				for (auto& input : node.inputs)
				{
					auto alpha = ImGui::GetStyle().Alpha;
					if (newLinkPin && !canCreateLink(newLinkPin, &input) && &input != newLinkPin)
						alpha = alpha * (48.0f / 255.0f);

					builder.Input(input.id);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
					drawPinIcon(input, isPinLinked(input.id), (int)(alpha * 255));
					ImGui::Spring(0);
					if (!input.name.empty())
					{
						ImGui::TextUnformatted(input.name.c_str());
						ImGui::Spring(0);
					}
					if (input.type == PinType::Bool)
					{
						ImGui::Button("Hello");
						ImGui::Spring(0);
					}
					ImGui::PopStyleVar();
					builder.EndInput();
				}

				if (isSimple)
				{
					builder.Middle();

					ImGui::Spring(1, 0);
					ImGui::TextUnformatted(node.name.c_str());
					ImGui::Spring(1, 0);
				}

				for (auto& output : node.outputs)
				{
					if (!isSimple && output.type == PinType::Delegate)
						continue;

					auto alpha = ImGui::GetStyle().Alpha;
					if (newLinkPin && !canCreateLink(newLinkPin, &output) && &output != newLinkPin)
						alpha = alpha * (48.0f / 255.0f);

					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
					builder.Output(output.id);
					if (output.type == PinType::String)
					{
						static char buffer[128] = "Edit Me\nMultiline!";
						static bool wasActive = false;

						ImGui::PushItemWidth(100.0f);
						ImGui::InputText("##edit", buffer, 127);
						ImGui::PopItemWidth();
						if (ImGui::IsItemActive() && !wasActive)
						{
							ed::EnableShortcuts(false);
							wasActive = true;
						}
						else if (!ImGui::IsItemActive() && wasActive)
						{
							ed::EnableShortcuts(true);
							wasActive = false;
						}
						ImGui::Spring(0);
					}
					if (!output.name.empty())
					{
						ImGui::Spring(0);
						ImGui::TextUnformatted(output.name.c_str());
					}
					ImGui::Spring(0);
					drawPinIcon(output, isPinLinked(output.id), (int)(alpha * 255));
					ImGui::PopStyleVar();
					builder.EndOutput();
				}

				builder.End();
			}

			for (auto& link : m_Links)
				ed::Link(link.id, link.startPinID, link.endPinID, link.color, 2.0f);

			if (!createNewNode)
			{
				if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
				{
					ed::PinId startPinId = 0, endPinId = 0;
					if (ed::QueryNewLink(&startPinId, &endPinId))
					{
						auto startPin = findPin(startPinId);
						auto endPin   = findPin(endPinId);

						newLinkPin = startPin ? startPin : endPin;

						if (startPin->kind == PinKind::Input)
						{
							std::swap(startPin, endPin);
							std::swap(startPinId, endPinId);
						}

						if (startPin && endPin)
						{
							if (endPin == startPin)
							{
								ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
							else if (endPin->kind == startPin->kind)
							{
								ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
							}
								//else if (endPin->node == startPin->node)
								//{
								//    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
								//    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
								//}
							else if (endPin->type != startPin->type)
							{
								ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
							}
							else
							{
								if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
								{
									m_Links.emplace_back(Link(getNextId(), startPinId, endPinId));
									m_Links.back().color = getIconColor(startPin->type);
								}
							}
						}
					}

					ed::PinId pinId = 0;
					if (ed::QueryNewNode(&pinId))
					{
						newLinkPin = findPin(pinId);

						if (ed::AcceptNewItem())
						{
							createNewNode  = true;
							newNodeLinkPin = findPin(pinId);
							newLinkPin = nullptr;
							ed::Suspend();
							ImGui::OpenPopup("Create New Node");
							ed::Resume();
						}
					}
				}
				else
					newLinkPin = nullptr;

				ed::EndCreate();

				if (ed::BeginDelete())
				{
					ed::LinkId linkId = 0;
					while (ed::QueryDeletedLink(&linkId))
					{
						if (ed::AcceptDeletedItem())
						{
							auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto& link) { return link.id == linkId; });
							if (id != m_Links.end()) {
								m_Links.erase(id);
							}
						}
					}

					ed::NodeId nodeId = 0;
					while (ed::QueryDeletedNode(&nodeId))
					{
						if (ed::AcceptDeletedItem())
						{
							auto id = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](auto& node) { return node.id == nodeId; });
							if (id != m_Nodes.end())
								m_Nodes.erase(id);
						}
					}
				}
				ed::EndDelete();
			}

			ImGui::SetCursorScreenPos(cursorTopLeft);
		}

		auto openPopupPosition = ImGui::GetMousePos();
		ed::Suspend();
		if (ed::ShowBackgroundContextMenu()) {
			ImGui::OpenPopup("Create New Node");
			newNodeLinkPin = nullptr;
		}
		ed::Resume();

		ed::Suspend();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
		if (ImGui::BeginPopup("Create New Node"))
		{
			auto newNodePostion = openPopupPosition;
			//ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

			//auto drawList = ImGui::GetWindowDrawList();
			//drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

			Node* node = nullptr;

			if (ImGui::MenuItem("Float"))
				node = spawnFloatNode();
			if (ImGui::MenuItem("Float2"))
				node = spawnFloat2Node();
			if (ImGui::MenuItem("Float3"))
				node = spawnFloat3Node();
			if (ImGui::MenuItem("Float4"))
				node = spawnFloat4Node();

			ImGui::Separator();

			if (ImGui::MenuItem("Texture2D"))
				node = spawnTextureNode();

			ImGui::Separator();

			if(ImGui::BeginMenu("Add")) {
				if (ImGui::MenuItem("Float + Float"))
					node = spawnAddFloatNode();
				if (ImGui::MenuItem("Float + Float2"))
					node = spawnAddFloatFloat2Node();
				if (ImGui::MenuItem("Float + Float3"))
					node = spawnAddFloatFloat3Node();
				if (ImGui::MenuItem("Float + Float4"))
					node = spawnAddFloatFloat4Node();
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Multiply")) {
				if (ImGui::MenuItem("Float * Float"))
					node = spawnMultiplyFloatNode();
				if (ImGui::MenuItem("Float * Float2"))
					node = spawnMultiplyFloatFloat2Node();
				if (ImGui::MenuItem("Float * Float3"))
					node = spawnMultiplyFloatFloat3Node();
				if (ImGui::MenuItem("Float * Float4"))
					node = spawnMultiplyFloatFloat4Node();
				ImGui::EndMenu();
			}

			if (node)
			{
				buildNodes();

				createNewNode = false;

				ed::SetNodePosition(node->id, newNodePostion);

				if (auto startPin = newNodeLinkPin)
				{
					auto& pins = startPin->kind == PinKind::Input ? node->outputs : node->inputs;

					for (auto& pin : pins)
					{
						if (canCreateLink(startPin, &pin))
						{
							auto endPin = &pin;
							if (startPin->kind == PinKind::Input)
								std::swap(startPin, endPin);

							m_Links.emplace_back(Link(getNextId(), startPin->id, endPin->id));
							m_Links.back().color = getIconColor(startPin->type);

							break;
						}
					}
				}
			}

			ImGui::EndPopup();
		}
		else
			createNewNode = false;
		ImGui::PopStyleVar();
		ed::Resume();

		ed::End();
	}

	Node* BlueprintBuilder::spawnFloatNode() {
		m_Nodes.emplace_back(getNextId(), "Float");
		Node& node = m_Nodes.back();
		node.outputs.emplace_back(getNextId(), "Value", PinType::Float);
		node.drawContent = [&]() {
			float* data = (float*) node.data;
			ImGui::SetNextItemWidth(64.0f);
			ImGui::DragFloat("", &data[0]);
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnFloat2Node() {
		m_Nodes.emplace_back(getNextId(), "Float2 Vector");
		Node& node = m_Nodes.back();
		node.outputs.emplace_back(getNextId(), "", PinType::Float2);
		node.drawContent = [&]() {
			float* data = (float*) node.data;
			ImGui::BeginVertical("Float2");
			{
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("X", &data[0]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("Y", &data[1]);
			}
			ImGui::EndVertical();
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnFloat3Node() {
		m_Nodes.emplace_back(getNextId(), "Float3 Vector");
		Node& node = m_Nodes.back();
		node.outputs.emplace_back(getNextId(), "", PinType::Float3);
		node.drawContent = [&]() {
			float* data = (float*) node.data;
			ImGui::BeginVertical("Float3");
			{
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("X", &data[0]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("Y", &data[1]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("Z", &data[2]);
			}
			ImGui::EndVertical();
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnFloat4Node() {
		m_Nodes.emplace_back(getNextId(), "Float4 Vector");
		Node& node = m_Nodes.back();
		node.outputs.emplace_back(getNextId(), "", PinType::Float4);
		node.drawContent = [&]() {
			float* data = (float*) node.data;
			ImGui::BeginVertical("Float4");
			{
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("X", &data[0]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("Y", &data[1]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("Z", &data[2]);
				ImGui::SetNextItemWidth(64.0f);
				ImGui::DragFloat("W", &data[3]);
			}
			ImGui::EndVertical();
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTextureNode() {
		m_Nodes.emplace_back(getNextId(), "Texture 2D");
		Node& node = m_Nodes.back();
		node.outputs.emplace_back(getNextId(), "", PinType::Texture2D);
		node.drawContent = [&]() {
			char* data = (char*) node.data;
			ImGui::BeginVertical("Texture 2D");
			{
				String name = Files::getName(data);
				ImGui::SetNextItemWidth(128.0f);
				ImGui::Text(name.c_str(), 256);
				if(ImGui::Button("...")) {
					Optional<String> file = UI::FileDialog::open(".png;.jpg;.jpeg;.gif;.bmp");
					if(file.has_value()) {
						try {
							Ref<Texture2D> texture = Assets::textures().load(file.value());
							memset(data, 0, 256);
							memcpy(data, texture->name().c_str(), std::min((size_t)256, texture->name().size()));
						} catch(...) {
						}
					}
				}
			}
			ImGui::EndVertical();
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnAddFloatNode() {
		m_Nodes.emplace_back(getNextId(), "Float + Float");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float);
		node.output = [&]() -> int8_t* {

			float a = 0;
			float b = 0;

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					b = ((float*)input->output())[0];
				}
			}

			float* data = (float*)node.data;
			data[0] = a + b;

			return node.data;
		};

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnAddFloatFloat2Node() {
		m_Nodes.emplace_back(getNextId(), "Float + Float2");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float2);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float2);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector2 b = {0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector2* inputData = ((Vector2*)input->output());
					b = inputData[0];
				}
			}

			Vector2* data = (Vector2*)node.data;
			data[0] = a + b;

			return node.data;
		};

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnAddFloatFloat3Node() {
		m_Nodes.emplace_back(getNextId(), "Float + Float3");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float3);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float3);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector3 b = {0, 0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector3* inputData = ((Vector3*)input->output());
					b = inputData[0];
				}
			}

			Vector3* data = (Vector3*)node.data;
			data[0] = a + b;

			return node.data;
		};

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnAddFloatFloat4Node() {
		m_Nodes.emplace_back(getNextId(), "Float + Float4");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float4);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float4);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector4 b = {0, 0, 0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector4* inputData = ((Vector4*)input->output());
					b = inputData[0];
				}
			}

			Vector4* data = (Vector4*)node.data;
			data[0] = a + b;

			return node.data;
		};

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMultiplyFloatNode() {
		m_Nodes.emplace_back(getNextId(), "Float * Float");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float);
		node.output = [&]() -> int8_t* {

			float a = 0;
			float b = 0;

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					b = ((float*)input->output())[0];
				}
			}

			float* data = (float*)node.data;
			data[0] = a * b;

			return node.data;
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMultiplyFloatFloat2Node() {
		m_Nodes.emplace_back(getNextId(), "Float * Float2");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float2);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float2);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector2 b = {0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector2* inputData = ((Vector2*)input->output());
					b = inputData[0];
				}
			}

			Vector2* data = (Vector2*)node.data;
			data[0] = a * b;

			return node.data;
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMultiplyFloatFloat3Node() {
		m_Nodes.emplace_back(getNextId(), "Float * Float3");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float3);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float3);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector3 b = {0, 0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector3* inputData = ((Vector3*)input->output());
					b = inputData[0];
				}
			}

			Vector3* data = (Vector3*)node.data;
			data[0] = a * b;

			return node.data;
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMultiplyFloatFloat4Node() {
		m_Nodes.emplace_back(getNextId(), "Float * Float4");
		Node& node = m_Nodes.back();
		node.inputs.emplace_back(getNextId(), "A", PinType::Float);
		node.inputs.emplace_back(getNextId(), "B", PinType::Float4);
		node.outputs.emplace_back(getNextId(), "Result", PinType::Float4);
		node.output = [&]() -> int8_t* {

			float a = 0;
			Vector4 b = {0, 0, 0, 0};

			Link* linkA = findLinkThatEndsAt(node.inputs[0].id);
			Link* linkB = findLinkThatEndsAt(node.inputs[1].id);

			if(linkA != nullptr) {
				Node* input = findPin(linkA->startPinID)->node;
				if(input) {
					a = ((float*)input->output())[0];
				}
			}

			if(linkB != nullptr) {
				Node* input = findPin(linkB->startPinID)->node;
				if(input) {
					Vector4* inputData = ((Vector4*)input->output());
					b = inputData[0];
				}
			}

			Vector4* data = (Vector4*)node.data;
			data[0] = a * b;

			return node.data;
		};
		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnInputActionNode() {
		m_Nodes.emplace_back(getNextId(), "InputAction Fire", ImColor(255, 128, 128));
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Delegate);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Pressed", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Released", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnBranchNode() {
		m_Nodes.emplace_back(getNextId(), "Branch");
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Condition", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(getNextId(), "True", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "False", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnDoNNode() {
		m_Nodes.emplace_back(getNextId(), "Do N");
		m_Nodes.back().inputs.emplace_back(getNextId(), "Enter", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "N", PinType::Int);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Reset", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Exit", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Counter", PinType::Int);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnOutputActionNode() {
		m_Nodes.emplace_back(getNextId(), "OutputAction");
		m_Nodes.back().inputs.emplace_back(getNextId(), "Sample", PinType::Float);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Condition", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Event", PinType::Delegate);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnPrintStringNode() {
		m_Nodes.emplace_back(getNextId(), "Print String");
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "In String", PinType::String);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMessageNode() {
		m_Nodes.emplace_back(getNextId(), "", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().outputs.emplace_back(getNextId(), "Message", PinType::String);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnSetTimerNode() {
		m_Nodes.emplace_back(getNextId(), "Set Timer", ImColor(128, 195, 248));
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Object", PinType::Object);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Function Name", PinType::Function);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Time", PinType::Float);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Looping", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnLessNode() {
		m_Nodes.emplace_back(getNextId(), "<", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Float);
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Float);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnWeirdNode() {
		m_Nodes.emplace_back(getNextId(), "o.O", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Float);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTraceByChannelNode() {
		m_Nodes.emplace_back(getNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Start", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "End", PinType::Int);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Trace Channel", PinType::Float);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Trace Complex", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Actors to Ignore", PinType::Int);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Draw Debug Type", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(getNextId(), "Ignore Self", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Out Hit", PinType::Float);
		m_Nodes.back().outputs.emplace_back(getNextId(), "Return Value", PinType::Bool);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeSequenceNode() {
		m_Nodes.emplace_back(getNextId(), "Sequence");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeTaskNode() {
		m_Nodes.emplace_back(getNextId(), "Move To");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeTask2Node() {
		m_Nodes.emplace_back(getNextId(), "Random Wait");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnComment() {
		m_Nodes.emplace_back(getNextId(), "Test Comment");
		m_Nodes.back().type = NodeType::Comment;
		m_Nodes.back().size = ImVec2(300, 200);

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnHoudiniTransformNode() {
		m_Nodes.emplace_back(getNextId(), "Transform");
		m_Nodes.back().type = NodeType::Houdini;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnHoudiniGroupNode() {
		m_Nodes.emplace_back(getNextId(), "Group");
		m_Nodes.back().type = NodeType::Houdini;
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(getNextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(getNextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Icon BlueprintBuilder::getHeaderBackground() {
		return m_HeaderBackground;
	}

	Icon BlueprintBuilder::getSaveIcon() {
		return m_SaveIcon;
	}

	Icon BlueprintBuilder::getRestoreIcon() {
		return m_RestoreIcon;
	}

	static ImVec2 sumImVec2(const ImVec2& v1, const ImVec2& v2) {
		return ImVec2(v1.x + v2.x, v1.y + v2.y);
	}

	bool BlueprintBuilder::splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1,
									float min_size2, float splitter_long_axis_size) {
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = sumImVec2(window->DC.CursorPos, (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1)));
		bb.Max = sumImVec2(bb.Min, CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f));
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	ImColor BlueprintBuilder::getIconColor(PinType type) {
		switch (type) {
			default:
			case PinType::Flow:     return ImColor(255, 255, 255);
			case PinType::Bool:     return ImColor(220,  48,  48);
			case PinType::Int:      return ImColor( 68, 201, 156);
			case PinType::Float:    return ImColor(147, 226,  74);
			case PinType::String:   return ImColor(124,  21, 153);
			case PinType::Object:
			case PinType::Float2:
			case PinType::Float3:
			case PinType::Float4:
				return ImColor( 51, 150, 215);
			case PinType::Function: return ImColor(218,   0, 183);
			case PinType::Delegate:
			case PinType::Texture2D:
				return ImColor(255,  48,  48);
		}
	}

	void BlueprintBuilder::drawPinIcon(const Pin& pin, bool connected, int alpha) {
		IconType iconType;
		ImColor  color = getIconColor(pin.type);
		color.Value.w = alpha / 255.0f;
		switch (pin.type) {
			case PinType::Flow:     iconType = IconType::Flow;   break;
			case PinType::Bool:     iconType = IconType::Circle; break;
			case PinType::Int:      iconType = IconType::Circle; break;
			case PinType::Float:    iconType = IconType::Circle; break;
			case PinType::String:   iconType = IconType::Circle; break;
			case PinType::Object:   iconType = IconType::Circle; break;
			case PinType::Function: iconType = IconType::Circle; break;
			case PinType::Delegate: iconType = IconType::Square; break;
			default: iconType = IconType::Circle;
		}

		ax::Widgets::Icon(ImVec2(m_PinIconSize, m_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
	}

	void BlueprintBuilder::buildNodes() {
		for (Node& node : m_Nodes)
			buildNode(&node);
	}

	void BlueprintBuilder::setHeaderBackground(Icon icon) {
		m_HeaderBackground = icon;
	}

	void BlueprintBuilder::reset() {
		std::remove_if(m_Nodes.begin(), m_Nodes.end(), [](const Node& node) -> bool {return node.deletable;});
		std::remove_if(m_Links.begin(), m_Links.end(), [&](const Link& link) -> bool {
			const auto* startPin = findPin(link.startPinID);
			const auto* endPin = findPin(link.endPinID);
			if(startPin && startPin->node->deletable) return true;
			if(endPin && endPin->node->deletable) return true;
			return false;
		});
	}

	Link* BlueprintBuilder::findLinkThatEndsAt(ed::PinId endPin) {
		for(uint32_t i = 0;i < m_Links.size();++i) {
			if(m_Links[i].endPinID == endPin) return &m_Links[i];
		}
		return nullptr;
	}
}
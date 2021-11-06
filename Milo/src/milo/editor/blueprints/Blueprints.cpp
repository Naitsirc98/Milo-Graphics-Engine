#include "milo/editor/blueprints/Blueprints.h"

namespace milo::bp {

	BlueprintBuilder::BlueprintBuilder() {

	}

	BlueprintBuilder::~BlueprintBuilder() {

	}

	int32_t BlueprintBuilder::nextId() {
		return m_NextId++;
	}

	ed::LinkId BlueprintBuilder::getNextLinkId() {
		return ed::LinkId(nextId());
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

	Node* BlueprintBuilder::spawnInputActionNode() {
		m_Nodes.emplace_back(nextId(), "InputAction Fire", ImColor(255, 128, 128));
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Delegate);
		m_Nodes.back().outputs.emplace_back(nextId(), "Pressed", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "Released", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnBranchNode() {
		m_Nodes.emplace_back(nextId(), "Branch");
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "Condition", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(nextId(), "True", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "False", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnDoNNode() {
		m_Nodes.emplace_back(nextId(), "Do N");
		m_Nodes.back().inputs.emplace_back(nextId(), "Enter", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "N", PinType::Int);
		m_Nodes.back().inputs.emplace_back(nextId(), "Reset", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "Exit", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "Counter", PinType::Int);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnOutputActionNode() {
		m_Nodes.emplace_back(nextId(), "OutputAction");
		m_Nodes.back().inputs.emplace_back(nextId(), "Sample", PinType::Float);
		m_Nodes.back().outputs.emplace_back(nextId(), "Condition", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(nextId(), "Event", PinType::Delegate);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnPrintStringNode() {
		m_Nodes.emplace_back(nextId(), "Print String");
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "In String", PinType::String);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnMessageNode() {
		m_Nodes.emplace_back(nextId(), "", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().outputs.emplace_back(nextId(), "Message", PinType::String);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnSetTimerNode() {
		m_Nodes.emplace_back(nextId(), "Set Timer", ImColor(128, 195, 248));
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "Object", PinType::Object);
		m_Nodes.back().inputs.emplace_back(nextId(), "Function Name", PinType::Function);
		m_Nodes.back().inputs.emplace_back(nextId(), "Time", PinType::Float);
		m_Nodes.back().inputs.emplace_back(nextId(), "Looping", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnLessNode() {
		m_Nodes.emplace_back(nextId(), "<", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Float);
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Float);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnWeirdNode() {
		m_Nodes.emplace_back(nextId(), "o.O", ImColor(128, 195, 248));
		m_Nodes.back().type = NodeType::Simple;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Float);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Float);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTraceByChannelNode() {
		m_Nodes.emplace_back(nextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "Start", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "End", PinType::Int);
		m_Nodes.back().inputs.emplace_back(nextId(), "Trace Channel", PinType::Float);
		m_Nodes.back().inputs.emplace_back(nextId(), "Trace Complex", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(nextId(), "Actors to Ignore", PinType::Int);
		m_Nodes.back().inputs.emplace_back(nextId(), "Draw Debug Type", PinType::Bool);
		m_Nodes.back().inputs.emplace_back(nextId(), "Ignore Self", PinType::Bool);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "Out Hit", PinType::Float);
		m_Nodes.back().outputs.emplace_back(nextId(), "Return Value", PinType::Bool);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeSequenceNode() {
		m_Nodes.emplace_back(nextId(), "Sequence");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeTaskNode() {
		m_Nodes.emplace_back(nextId(), "Move To");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnTreeTask2Node() {
		m_Nodes.emplace_back(nextId(), "Random Wait");
		m_Nodes.back().type = NodeType::Tree;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnComment() {
		m_Nodes.emplace_back(nextId(), "Test Comment");
		m_Nodes.back().type = NodeType::Comment;
		m_Nodes.back().size = ImVec2(300, 200);

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnHoudiniTransformNode() {
		m_Nodes.emplace_back(nextId(), "Transform");
		m_Nodes.back().type = NodeType::Houdini;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);

		buildNode(&m_Nodes.back());

		return &m_Nodes.back();
	}

	Node* BlueprintBuilder::spawnHoudiniGroupNode() {
		m_Nodes.emplace_back(nextId(), "Group");
		m_Nodes.back().type = NodeType::Houdini;
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().inputs.emplace_back(nextId(), "", PinType::Flow);
		m_Nodes.back().outputs.emplace_back(nextId(), "", PinType::Flow);

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
			case PinType::Object:   return ImColor( 51, 150, 215);
			case PinType::Function: return ImColor(218,   0, 183);
			case PinType::Delegate: return ImColor(255,  48,  48);
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
			default: return;
		}

		ax::Widgets::Icon(ImVec2(m_PinIconSize, m_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
	}
}
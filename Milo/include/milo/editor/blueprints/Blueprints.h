#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/textures/Texture.h"

#include <imgui_node_editor.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "builders.h"
#include "drawing.h"
#include "widgets.h"

namespace milo::bp {

	namespace ed = ax::NodeEditor;
	namespace util = ax::NodeEditor::Utilities;

	using namespace ax;

	using ax::Widgets::IconType;

	enum class PinType {
		Flow,
		Bool,
		Int,
		Float,
		Float2,
		Float3,
		Float4,
		Texture2D,
		String,
		Object,
		Function,
		Delegate
	};

	enum class PinKind {
		Output,
		Input
	};

	enum class NodeType {
		Blueprint,
		Simple,
		Tree,
		Comment,
		Houdini
	};

	struct Node;

	struct Pin {

		ed::PinId id;
		Node* node;
		String name;
		PinType type;
		PinKind kind;

		Pin(int id, const char* name, PinType type):
				id(id), node(nullptr), name(name), type(type), kind(PinKind::Input)
		{
		}
	};

	struct Node {

		ed::NodeId id;
		String name;
		ArrayList<Pin> inputs;
		ArrayList<Pin> outputs;
		ImColor color;
		NodeType type;
		ImVec2 size;

		bool deletable = true;

		String state;
		String savedState;

		Function<void> drawContent = [](){};
		Function<int8_t*> output = [&]() {return data;};

		void* userData{nullptr};

		int8_t data[256]{0};

		Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)):
				id(id), name(name), color(color), type(NodeType::Blueprint), size(0, 0)
		{
		}
	};

	struct Link {

		ed::LinkId id;
		ed::PinId startPinID;
		ed::PinId endPinID;
		ImColor color;

		Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId):
				id(id), startPinID(startPinId), endPinID(endPinId), color(255, 255, 255)
		{
		}
	};

	struct NodeIdLess {

		bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const {
			return lhs.AsPointer() < rhs.AsPointer();
		}
	};

	struct Icon {
		Ref<Texture2D> texture{nullptr};
		ImTextureID iconId{nullptr};
	};

	class BlueprintBuilder {
	private:
		int32_t m_NextId = 1;
		int32_t m_PinIconSize = 24;
		ArrayList<Node> m_Nodes;
		ArrayList<Link> m_Links;
		Icon m_HeaderBackground;
		Icon m_SaveIcon;
		Icon m_RestoreIcon;
		const float m_TouchTime = 1.0f;
		std::map<ed::NodeId, float, NodeIdLess> m_NodeTouchTime;
	public:
		BlueprintBuilder();
		~BlueprintBuilder();
	public:
		int32_t getNextId();
		ed::LinkId getNextLinkId();
		void touchNode(ed::NodeId id);
		float getTouchProgress(ed::NodeId id);
		void updateTouch();
		Node* findNode(ed::NodeId id);
		Link* findLink(ed::LinkId id);
		Pin* findPin(ed::PinId id);
		bool isPinLinked(ed::PinId id);
		bool canCreateLink(Pin* a, Pin* b);
		const ArrayList<Link>& links() const;
		const ArrayList<Node>& nodes() const;
		void addNode(Node node);
		void buildNode(Node* node);
		void draw();
		Node* spawnFloatNode();
		Node* spawnFloat2Node();
		Node* spawnFloat3Node();
		Node* spawnFloat4Node();
		Node* spawnTextureNode();
		Node* spawnAddFloatNode();
		Node* spawnAddFloatFloat2Node();
		Node* spawnAddFloatFloat3Node();
		Node* spawnAddFloatFloat4Node();
		Node* spawnMultiplyFloatNode();
		Node* spawnMultiplyFloatFloat2Node();
		Node* spawnMultiplyFloatFloat3Node();
		Node* spawnMultiplyFloatFloat4Node();
		Node* spawnInputActionNode();
		Node* spawnBranchNode();
		Node* spawnDoNNode();
		Node* spawnOutputActionNode();
		Node* spawnPrintStringNode();
		Node* spawnMessageNode();
		Node* spawnSetTimerNode();
		Node* spawnLessNode();
		Node* spawnWeirdNode();
		Node* spawnTraceByChannelNode();
		Node* spawnTreeSequenceNode();
		Node* spawnTreeTaskNode();
		Node* spawnTreeTask2Node();
		Node* spawnComment();
		Node* spawnHoudiniTransformNode();
		Node* spawnHoudiniGroupNode();
		Icon getHeaderBackground();
		void setHeaderBackground(Icon icon);
		Icon getSaveIcon();
		Icon getRestoreIcon();
		void drawPinIcon(const Pin& pin, bool connected, int alpha);
		bool splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1,
					  float min_size2, float splitter_long_axis_size = -1.0f);
		ImColor getIconColor(PinType type);

		void buildNodes();
		void reset();
		Link* findLinkThatEndsAt(ed::PinId endPin);
	};
}
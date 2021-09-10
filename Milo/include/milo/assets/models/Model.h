#pragma once

#include "milo/assets/meshes/Mesh.h"
#include "milo/assets/materials/Material.h"

#define MILO_MODEL_NODE_MAX_NUM_CHILDREN 32

namespace milo {

	class Model : public Asset {
		friend class ModelManager;
		friend class ModelLoader;
		friend class AssimpModelLoader;
	public:
		struct Node {
			friend class Model;

			Model* model{nullptr};
			uint32_t index{UINT32_MAX};
			String name;
			Matrix4 transform = Matrix4(1.0f);
			Mesh* mesh{nullptr};
			Material* material{nullptr};
			ArrayList<uint32_t> children;

		private:
			Node() = default;
		};
	public:
		inline static const Node Empty{nullptr, UINT32_MAX, "", Matrix4(1.0f), nullptr, nullptr, {}};
	private:
		ArrayList<Node> m_Nodes;
	private:
		Model() = default;
		~Model() override = default;
	public:
		Model(const Model& other) = delete;
		Model& operator=(const Model& other) = delete;
		inline Node root() const {return m_Nodes[0];}
		inline Node get(uint32_t index) const {return m_Nodes[index];}
		inline Node get(const String& name) const {
			for(const Node& node : m_Nodes) {
				if(node.name == name) return node;
			}
			return Empty;
		}
		inline Node operator[](uint32_t index) const {
			return m_Nodes[index];
		}
		inline const ArrayList<Node>& nodes() const {return m_Nodes;}
		inline uint32_t size() const {return m_Nodes.size();}

		inline Node createNode() {
			Node node{};
			node.model = this;
			node.index = m_Nodes.size();
			m_Nodes.push_back(node);
			return node;
		}
	};
}
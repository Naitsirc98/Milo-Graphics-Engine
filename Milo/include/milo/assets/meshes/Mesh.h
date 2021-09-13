#pragma once

#include "milo/assets/Asset.h"
#include "milo/graphics/buffer/Buffer.h"

namespace milo {

	struct Vertex {
		Vector3 position = {0, 0, 0};
		Vector3 normal = {0, 0, 0};
		Vector2 uv = {0, 0};
	};

	class Mesh : public Asset {
		friend class MeshManager;
		friend class ObjMeshLoader;
		friend class AssimpLoader;
		friend class AssimpModelLoader;
	public:
		class GraphicsBuffers { // Implemented by the APIs
			friend class Mesh;
			friend class MeshManager;
		protected:
			virtual ~GraphicsBuffers() = default;
		private:
			static GraphicsBuffers* create(const ArrayList<Vertex>& vertices, const ArrayList<uint32_t>& indices);
		};
	private:
		ArrayList<Vertex> m_Vertices;
		ArrayList<uint32_t> m_Indices;
		GraphicsBuffers* m_Buffers = nullptr;
		// TODO: bounds (AABB, SPHERE, ETC)
	private:
		explicit Mesh(String filename);
		~Mesh();
	public:
		const ArrayList<Vertex>& vertices() const;
		const ArrayList<uint32_t>& indices() const;
		GraphicsBuffers* buffers() const;
	};

	class VertexList {
	private:
		const ArrayList<Vertex>& m_Vertices;
		const ArrayList<uint32_t>& m_Indices;
	public:
		VertexList(Mesh* mesh) : VertexList(mesh->vertices(), mesh->indices()) {}
		VertexList(const ArrayList<Vertex>& vertices, const ArrayList<uint32_t>& indices)
			: m_Vertices(vertices), m_Indices(indices) {
		}

		~VertexList() = default;

		Vertex operator[](uint32_t index) const {
			return m_Vertices[(m_Indices.empty() ? index : m_Indices[index])];
		}

		size_t size() const {
			return m_Indices.empty() ? m_Vertices.size() : m_Indices.size();
		}

		ArrayList<Vertex>::const_iterator begin() const {
			return m_Vertices.cbegin();
		}

		ArrayList<Vertex>::const_iterator end() const {
			return m_Vertices.cend();
		}
	};
}
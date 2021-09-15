#pragma once

#include "milo/assets/Asset.h"
#include "milo/graphics/buffer/Buffer.h"
#include "milo/graphics/Vertex.h"

namespace milo {

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
		GraphicsBuffers* m_Buffers{nullptr};
		BoundingVolume* m_BoundingVolume{nullptr};
		bool m_CanBeCulled{true};
	private:
		explicit Mesh(String filename);
		~Mesh() override;
	public:
		const ArrayList<Vertex>& vertices() const;
		const ArrayList<uint32_t>& indices() const;
		GraphicsBuffers* buffers() const;
		const BoundingVolume& boundingVolume() const;
		bool canBeCulled() const;
		void setCanBeCulled(bool value);
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

		inline const Vertex& operator[](uint32_t index) const {
			return m_Vertices[m_Indices.empty() ? index : m_Indices[index]];
		}

		inline size_t size() const {
			return m_Indices.empty() ? m_Vertices.size() : m_Indices.size();
		}
	};
}
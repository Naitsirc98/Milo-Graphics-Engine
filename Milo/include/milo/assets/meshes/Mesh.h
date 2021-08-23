#pragma once

#include "milo/graphics/buffer/Buffer.h"

namespace milo {

	struct Vertex {
		Vector3 position = {0, 0, 0};
		Vector3 normal = {0, 0, 0};
		Vector2 uv = {0, 0};
	};

	class Mesh {
		friend class MeshManager;
		friend class ObjMeshLoader;
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
		String m_Filename;
		ArrayList<Vertex> m_Vertices;
		ArrayList<uint32_t> m_Indices;
		GraphicsBuffers* m_Buffers = nullptr;
		// TODO: bounds (AABB, SPHERE, ETC)
	private:
		explicit Mesh(String filename);
		~Mesh();
	public:
		const String& filename() const;
		const ArrayList<Vertex>& vertices() const;
		const ArrayList<uint32_t>& indices() const;
		GraphicsBuffers* buffers() const;
	};

}
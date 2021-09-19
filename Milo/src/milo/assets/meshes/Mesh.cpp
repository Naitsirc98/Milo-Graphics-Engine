#include "milo/assets/meshes/Mesh.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include <assimp/postprocess.h>

namespace milo {

	const int ASSIMP_FLAGS = aiProcess_OptimizeGraph
							 | aiProcess_CalcTangentSpace
							 | aiProcess_Triangulate
							 //| aiProcess_GenSmoothNormals
							 //| aiProcess_GenUVCoords
							 //| aiProcess_FlipUVs
							 | aiProcess_JoinIdenticalVertices;

	Mesh::Mesh(String filename) {
		m_Filename = std::move(filename);
	}

	Mesh::~Mesh() {
		DELETE_PTR(m_Buffers);
		DELETE_PTR(m_BoundingVolume);
	}

	const ArrayList<milo::Vertex>& Mesh::vertices() const {
		return m_Vertices;
	}

	const ArrayList<uint32_t>& Mesh::indices() const {
		return m_Indices;
	}

	Mesh::GraphicsBuffers* Mesh::buffers() const {
		return m_Buffers;
	}

	const BoundingVolume& Mesh::boundingVolume() const {
		return *m_BoundingVolume;
	}

	bool Mesh::canBeCulled() const {
		return m_CanBeCulled;
	}

	void Mesh::setCanBeCulled(bool value) {
		m_CanBeCulled = value;
	}

	// ====

	Mesh::GraphicsBuffers* Mesh::GraphicsBuffers::create(const ArrayList<Vertex>& vertices, const ArrayList<uint32_t>& indices) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanMeshBuffers(vertices, indices);
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}
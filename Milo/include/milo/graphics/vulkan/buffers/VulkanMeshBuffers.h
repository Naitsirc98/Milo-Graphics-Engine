#pragma once

#include "milo/assets/meshes/Mesh.h"
#include "VulkanBuffer.h"

namespace milo {

	class VulkanMeshBuffers : public Mesh::GraphicsBuffers {
	private:
		VulkanBuffer* m_VertexBuffer = nullptr;
		VulkanBuffer* m_IndexBuffer = nullptr;
	public:
		VulkanMeshBuffers(const ArrayList<Vertex>& vertices, const ArrayList<uint32_t>& indices);
		~VulkanMeshBuffers() override;
		VulkanBuffer* vertexBuffer() const;
		VulkanBuffer* indexBuffer() const;
	private:
		void createVertexBuffer(const ArrayList<Vertex>& vertices);
		void createIndexBuffer(const ArrayList<uint32_t>& indices);
	};

}
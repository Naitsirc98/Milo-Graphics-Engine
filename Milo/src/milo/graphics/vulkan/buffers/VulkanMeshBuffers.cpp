#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"

namespace milo {

	VulkanMeshBuffers::VulkanMeshBuffers(const ArrayList<Vertex>& vertices, const ArrayList<uint32_t>& indices) {
		createVertexBuffer(vertices);
		createIndexBuffer(indices);
	}

	VulkanMeshBuffers::~VulkanMeshBuffers() {
		DELETE_PTR(m_VertexBuffer);
		DELETE_PTR(m_IndexBuffer);
	}

	VulkanBuffer* VulkanMeshBuffers::vertexBuffer() const {
		return m_VertexBuffer;
	}

	VulkanBuffer* VulkanMeshBuffers::indexBuffer() const {
		return m_IndexBuffer;
	}

	void VulkanMeshBuffers::createVertexBuffer(const ArrayList<Vertex>& vertices) {

		m_VertexBuffer = VulkanBuffer::createVertexBuffer();

		Buffer::AllocInfo allocInfo = {};
		allocInfo.size = vertices.size() * sizeof(Vertex);
		allocInfo.data = vertices.data();

		m_VertexBuffer->allocate(allocInfo);
	}

	void VulkanMeshBuffers::createIndexBuffer(const ArrayList<uint32_t>& indices) {

		if(indices.empty()) return;

		m_IndexBuffer = VulkanBuffer::createIndexBuffer();

		Buffer::AllocInfo allocInfo = {};
		allocInfo.size = indices.size() * sizeof(uint32_t);
		allocInfo.data = indices.data();

		m_IndexBuffer->allocate(allocInfo);
	}
}
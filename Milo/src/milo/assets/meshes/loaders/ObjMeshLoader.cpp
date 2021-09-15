#include "milo/assets/meshes/loaders/ObjMeshLoader.h"
#include "milo/io/Files.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace milo {

	Mesh* ObjMeshLoader::load(const String& filename) {
		if(!Files::exists(filename)) throw MILO_RUNTIME_EXCEPTION(fmt::format("File {} does not exists", filename));
		if(Files::isDirectory(filename)) throw MILO_RUNTIME_EXCEPTION(fmt::format("{} is a directory", filename));

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		String warn;
		String err;

		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), nullptr, true);

		if (!warn.empty()) {
			Log::warn("{}: {}", filename, warn);
		}
		if (!err.empty()) {
			throw MILO_RUNTIME_EXCEPTION(fmt::format("Failed to load {}: {}", filename, err));
			//Log::error("{}: {}", filename, err);
			//return nullptr;
		}

		Mesh* mesh = new Mesh(filename);

		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t idxOffset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

				// Triangulate is on
				size_t fv = 3;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {

					tinyobj::index_t idx = shapes[s].mesh.indices[idxOffset + v];

					Vertex vertex = {};

					// Position
					vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0];
					vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1];
					vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2];
					// Normal
					vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
					vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
					vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];
					// UV
					vertex.texCoords.x = attrib.texcoords[2 * idx.texcoord_index + 0];
					vertex.texCoords.y = attrib.texcoords[2 * idx.texcoord_index + 1];

					mesh->m_Vertices.push_back(vertex);
					//mesh->m_Indices.push_back(idx.vertex_index);
				}
				idxOffset += fv;
			}

			mesh->m_Vertices.shrink_to_fit();
			//mesh->m_Indices.shrink_to_fit();

			return mesh;
		}
	}

}
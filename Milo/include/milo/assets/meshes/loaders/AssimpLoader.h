#pragma once

#include "milo/assets/meshes/MeshLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace milo {

	class AssimpLoader : public MeshLoader {
	private:
	public:
		AssimpLoader();
		~AssimpLoader();
	public:
		Mesh* load(const String& filename) override;
	private:
		void processMesh(const aiScene* scene, const aiNode* node, Mesh* outMesh);
		void processIndices(const aiMesh* aiMesh, Mesh* outMesh);
	};

}
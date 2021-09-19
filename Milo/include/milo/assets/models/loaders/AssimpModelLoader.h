#pragma once

#include "milo/assets/models/ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace milo {

	class AssimpModelLoader : public ModelLoader {
	private:
		String m_File;
		String m_Dir;
	public:
		Model* load(const String& filename) override;
	private:
		void processScene(const aiScene* scene, Model* outModel);
		void processNode(const aiScene* scene, const aiNode* node, Model::Node* outNode);
		void processMesh(const aiScene* scene, const aiMesh* mesh, Mesh* outMesh);
		void processIndices(const aiMesh* aiMesh, Mesh* outMesh);
		void processMaterial(const aiScene* scene, const aiMaterial* aiMaterial, Material* outMaterial);
		Ref<Texture2D> getTexture(const aiScene* aiScene, const aiMaterial* aiMaterial, aiTextureType type, PixelFormat format, bool* present = nullptr);

		void processNodeMeshes(const aiScene* aiScene, const aiNode* aiNode, Model::Node* outNode);
	};

}
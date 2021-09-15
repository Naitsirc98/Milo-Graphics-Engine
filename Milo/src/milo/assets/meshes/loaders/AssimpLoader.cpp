#include "milo/assets/meshes/loaders/AssimpLoader.h"
#include <assimp/postprocess.h>

namespace milo {

	AssimpLoader::AssimpLoader() {

	}

	AssimpLoader::~AssimpLoader() {

	}

	Mesh* AssimpLoader::load(const String& filename) {

		static const int ASSIMP_FLAGS = aiProcess_OptimizeMeshes
										| aiProcess_OptimizeGraph
										| aiProcess_Triangulate
										| aiProcess_CalcTangentSpace
										| aiProcess_JoinIdenticalVertices;

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filename.c_str(), ASSIMP_FLAGS);

		if(scene == nullptr || scene->mRootNode == nullptr) {
			throw MILO_RUNTIME_EXCEPTION(fmt::format("Failed to import file {}: {}", filename, importer.GetErrorString()));
		}

		Mesh* mesh = new Mesh(filename);

		// TODO: process subtree and multiple meshes in 1 node
		processMesh(scene, scene->mRootNode, mesh);

		return mesh;
	}

	void AssimpLoader::processMesh(const aiScene* scene, const aiNode* node, Mesh* outMesh) {

		if(node->mNumMeshes == 0) return;

		const aiMesh* aiMesh = scene->mMeshes[node->mMeshes[0]];

		outMesh->m_Vertices.reserve(aiMesh->mNumVertices);

		for(uint32_t i = 0;i < aiMesh->mNumVertices;++i) {

			const auto& position = aiMesh->mVertices[i];
			const auto& normal = aiMesh->mNormals[i];
			const auto& tangent = aiMesh->mTangents[i];
			const auto& biTangent = aiMesh->mBitangents[i];
			const auto& texCoords = aiMesh->mTextureCoords[0][i];

			Vertex vertex{};
			vertex.position = {position.x, position.y, position.z};
			vertex.normal = {normal.x, normal.y, normal.z};
			vertex.tangent = {tangent.x, tangent.y, tangent.z};
			vertex.biTangent = {biTangent.x, biTangent.y, biTangent.z};
			vertex.texCoords = {texCoords.x, texCoords.y};

			outMesh->m_Vertices.push_back(vertex);
		}

		processIndices(aiMesh, outMesh);
	}

	void AssimpLoader::processIndices(const aiMesh* aiMesh, Mesh* outMesh) {

		if(aiMesh->mNumFaces == 0) return;

		outMesh->m_Indices.reserve(aiMesh->mNumFaces * 3);

		for(uint32_t i = 0;i < aiMesh->mNumFaces;++i) {

			const aiFace& face = aiMesh->mFaces[i];

			outMesh->m_Indices.push_back(face.mIndices[0]);
			outMesh->m_Indices.push_back(face.mIndices[1]);
			outMesh->m_Indices.push_back(face.mIndices[2]);
		}
	}
}
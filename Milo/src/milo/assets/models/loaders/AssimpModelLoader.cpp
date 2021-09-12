#include "milo/assets/models/loaders/AssimpModelLoader.h"
#include "milo/assets/AssetManager.h"
#include "milo/io/Files.h"
#include <assimp/postprocess.h>

namespace milo {

	static const int ASSIMP_FLAGS = aiProcess_OptimizeMeshes
			//| aiProcess_OptimizeGraph
			| aiProcess_Triangulate
			//| aiProcess_FlipUVs
			| aiProcess_JoinIdenticalVertices;
			//| aiProcess_GenSmoothNormals;

	Model* AssimpModelLoader::load(const String& filename) {

		m_File = filename;
		m_Dir = Files::parentOf(filename);

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filename.c_str(), ASSIMP_FLAGS);

		if(scene == nullptr || scene->mRootNode == nullptr) {
			throw MILO_RUNTIME_EXCEPTION(fmt::format("Failed to import file {}: {}", filename, importer.GetErrorString()));
		}

		Model* model = new Model();

		processScene(scene, model);

		return model;
	}

	void AssimpModelLoader::processScene(const aiScene* aiScene, Model* outModel) {
		const aiNode* aiNode = aiScene->mRootNode;
		Model::Node* node = outModel->createNode();
		processNode(aiScene, aiNode, node);
	}

	void AssimpModelLoader::processNode(const aiScene* aiScene, const aiNode* aiNode, Model::Node* outNode) {

		outNode->name = aiNode->mName.C_Str();

		memcpy(&outNode->transform, &aiNode->mTransformation, sizeof(Matrix4));

		if(aiNode->mNumMeshes > 0) {
			const aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[0]];
			Mesh* mesh = Assets::meshes().find(aiMesh->mName.C_Str());

			if(mesh == nullptr) {
				mesh = new Mesh(m_File);
				mesh->m_Name = aiMesh->mName.C_Str();
				processMesh(aiScene, aiMesh, mesh);
				Assets::meshes().addMesh(mesh->name(), mesh);
			}
			outNode->mesh = mesh;

			Material* material = Assets::materials().getDefault();
			if(aiMesh->mMaterialIndex >= 0) {
				const aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
				material = Assets::materials().find(aiMaterial->GetName().C_Str());

				if(material == nullptr) {
					material = new Material(aiMaterial->GetName().C_Str(), m_File);
					processMaterial(aiScene, aiMaterial, material);
					Assets::materials().addMaterial(material->name(), material);
				}
			}
			outNode->material = material;
		}

		// Children
		for(uint32_t i = 0;i < aiNode->mNumChildren;++i) {
			const auto* aiChild = aiNode->mChildren[i];
			Model::Node* child = outNode->model->createNode();
			outNode->children.push_back(child->index);
			processNode(aiScene, aiChild, child);
		}
	}

	void AssimpModelLoader::processMesh(const aiScene* aiScene, const aiMesh* aiMesh, Mesh* outMesh) {

		outMesh->m_Vertices.reserve(aiMesh->mNumVertices);

		for(uint32_t i = 0;i < aiMesh->mNumVertices;++i) {

			const auto& position = aiMesh->mVertices[i];
			const auto& normal = aiMesh->mNormals[i];
			const auto& uv = aiMesh->mTextureCoords[0][i];

			Vertex vertex{};
			vertex.position = {position.x, position.y, position.z};
			vertex.normal = {normal.x, normal.y, normal.z};
			vertex.uv = {uv.x, uv.y};

			outMesh->m_Vertices.push_back(vertex);
		}

		processIndices(aiMesh, outMesh);
	}

	void AssimpModelLoader::processIndices(const aiMesh* aiMesh, Mesh* outMesh) {

		if(aiMesh->mNumFaces == 0) return;

		outMesh->m_Indices.reserve(aiMesh->mNumFaces * 3);

		for(uint32_t i = 0;i < aiMesh->mNumFaces;++i) {

			const aiFace& face = aiMesh->mFaces[i];

			outMesh->m_Indices.push_back(face.mIndices[0]);
			outMesh->m_Indices.push_back(face.mIndices[1]);
			outMesh->m_Indices.push_back(face.mIndices[2]);
		}
	}

	inline static void getColor(const aiMaterial* aiMaterial, const char* name, uint32_t type, uint32_t idx, Color& outColor) {
		ai_real color[3]{0, 0, 0};
		aiMaterial->Get(name, type, idx, color, nullptr);
		memcpy(&outColor, color, sizeof(float) * 3);
	}

	inline static void getFloat(const aiMaterial* aiMaterial, const char* name, uint32_t type, uint32_t idx, float& outFloat) {
		ai_real aiFloat;
		aiMaterial->Get(name, type, idx, &aiFloat, nullptr);
		outFloat = aiFloat;
	}

	Ref<Texture2D> AssimpModelLoader::getTexture(const aiScene* aiScene, const aiMaterial* aiMaterial, aiTextureType type, PixelFormat format) {

		if(aiMaterial->GetTextureCount(type) == 0) {
			if(type == aiTextureType_EMISSIVE) return Assets::textures().blackTexture();
			return Assets::textures().whiteTexture();
		}

		aiString path;
		aiMaterial->GetTexture(type, 0, &path);

		String textureFile = path.C_Str();

		if(!Files::isAbsolute(textureFile)) {
			textureFile = Files::append(m_Dir, textureFile);
		}

		return Assets::textures().load(textureFile, format);
	}

	void AssimpModelLoader::processMaterial(const aiScene* aiScene, const aiMaterial* aiMaterial, Material* outMaterial) {

		Material::Data& mat = outMaterial->m_Data;

		getColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, mat.albedo);
		getColor(aiMaterial, AI_MATKEY_COLOR_EMISSIVE, mat.emissiveColor);

		getFloat(aiMaterial, AI_MATKEY_METALLIC_FACTOR, mat.metallic);
		getFloat(aiMaterial, AI_MATKEY_OPACITY, mat.alpha);
		getFloat(aiMaterial, AI_MATKEY_ROUGHNESS_FACTOR, mat.roughness);

		outMaterial->m_AlbedoMap = getTexture(aiScene, aiMaterial, aiTextureType_DIFFUSE, PixelFormat::SRGBA);
		//outMaterial->m_AlbedoMap = getTexture(aiScene, aiMaterial, aiTextureType_BASE_COLOR, PixelFormat::RGBA8);
		outMaterial->m_EmissiveMap = getTexture(aiScene, aiMaterial, aiTextureType_EMISSIVE, PixelFormat::SRGBA);
		outMaterial->m_NormalMap = getTexture(aiScene, aiMaterial, aiTextureType_NORMALS, PixelFormat::SRGBA);
		outMaterial->m_MetallicMap = getTexture(aiScene, aiMaterial, aiTextureType_METALNESS, PixelFormat::SRGBA);
		outMaterial->m_RoughnessMap = getTexture(aiScene, aiMaterial, aiTextureType_DIFFUSE_ROUGHNESS, PixelFormat::SRGBA);
		outMaterial->m_OcclusionMap = getTexture(aiScene, aiMaterial, aiTextureType_AMBIENT_OCCLUSION, PixelFormat::SRGBA);
	}
}

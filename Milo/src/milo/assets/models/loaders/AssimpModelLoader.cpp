#include "milo/assets/models/loaders/AssimpModelLoader.h"
#include "milo/assets/AssetManager.h"
#include "milo/io/Files.h"
#include <assimp/postprocess.h>

namespace milo {

	Model* AssimpModelLoader::load(const String& filename) {

		m_File = filename;
		m_Dir = Files::parentOf(filename);
		m_LoadedTextureNames.clear();

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

		//if(outNode->parent >= 0) {
		//	const Model::Node* parent = outNode->model->get(outNode->parent);
		//	outNode->transform = parent->transform * outNode->transform;
		//}

		for(uint32_t i = 0;i < aiNode->mNumMeshes;++i) {
			processNodeMeshes(aiScene, aiNode, outNode);
		}

		// Children
		for(uint32_t i = 0;i < aiNode->mNumChildren;++i) {
			const auto* aiChild = aiNode->mChildren[i];
			Model::Node* child = outNode->model->createNode();
			outNode->children.push_back(child->index);
			child->parent = outNode->index;
			processNode(aiScene, aiChild, child);
		}
	}

	void AssimpModelLoader::processNodeMeshes(const aiScene* aiScene, const aiNode* aiNode, Model::Node* outNode) {

		for(uint32_t i = 0;i < aiNode->mNumMeshes;++i) {

			Model::Node* node = outNode->model->createNode();
			outNode->children.push_back(node->index);
			node->transform = outNode->transform;

			const aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
			Mesh* mesh = Assets::meshes().find(aiMesh->mName.C_Str());

			node->name = outNode->name + "[" + str(i) + "]";

			if(mesh == nullptr) {
				mesh = new Mesh(m_File);
				mesh->m_Name = aiMesh->mName.C_Str();
				processMesh(aiScene, aiMesh, mesh);
				Assets::meshes().addMesh(mesh->name(), mesh);
			}
			node->mesh = mesh;

			Material* material = Assets::materials().getDefault();
			if(aiMesh->mMaterialIndex >= 0) {
				const aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
				String materialName = str("M_") + node->name + str(i);
				material = Assets::materials().find(materialName);

				if(material == nullptr) {
					material = new Material(materialName, m_File);
					processMaterial(aiScene, aiMaterial, material);
					Assets::materials().addMaterial(material->name(), material);
				}
			}
			node->material = material;
		}
	}

	void AssimpModelLoader::processMesh(const aiScene* aiScene, const aiMesh* aiMesh, Mesh* outMesh) {

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
		ai_real color[4]{0, 0, 0, 0};
		aiMaterial->Get(name, type, idx, color, nullptr);
		memcpy(&outColor, color, sizeof(float) * 4);
	}

	inline static void getFloat(const aiMaterial* aiMaterial, const char* name, uint32_t type, uint32_t idx, float& outFloat) {
		ai_real aiFloat;
		aiMaterial->Get(name, type, idx, &aiFloat, nullptr);
		outFloat = aiFloat;
	}

	Ref<Texture2D> AssimpModelLoader::getTexture(const aiScene* aiScene, const aiMaterial* aiMaterial,
												 aiTextureType type, PixelFormat format, bool* present) {

		if(aiMaterial->GetTextureCount(type) == 0) {
			if(present != nullptr) {
				*present = false;
			}
			if(type == aiTextureType_EMISSIVE) return Assets::textures().blackTexture();
			return Assets::textures().whiteTexture();
		}

		aiString path;
		aiMaterial->GetTexture(type, 0, &path);

		String textureFile = path.C_Str();

		if(!Files::isAbsolute(textureFile)) {
			textureFile = Files::append(m_Dir, textureFile);
		}

		auto texture =  Assets::textures().load(textureFile, format);

		texture->setName(path.C_Str());

		if(present != nullptr) {
			*present = true;
		}

		m_LoadedTextureNames.insert(texture->name());

		return texture;
	}

	inline static bool containsAny(const String& s, const HashSet<String>& tokens) {
		for(const String& token : tokens) {
			if(s.find(token) != String::npos) return true;
		}
		return false;
	}

	void AssimpModelLoader::processMaterial(const aiScene* aiScene, const aiMaterial* aiMaterial, Material* outMaterial) {

		Material::Data& mat = outMaterial->m_Data;

		getColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, mat.albedo);
		getColor(aiMaterial, AI_MATKEY_COLOR_EMISSIVE, mat.emissiveColor);

		getFloat(aiMaterial, AI_MATKEY_METALLIC_FACTOR, mat.metallic);
		getFloat(aiMaterial, AI_MATKEY_OPACITY, mat.alpha);
		getFloat(aiMaterial, AI_MATKEY_ROUGHNESS_FACTOR, mat.roughness);

		bool hasAlbedoMap = false;
		bool hasEmissiveMap = false;
		bool hasNormalMap = false;
		bool hasMetallicMap = false;
		bool hasRoughnessMap = false;
		bool hasOcclusionMap = false;
		bool hasMetallicRoughnessMap = false;

		outMaterial->m_AlbedoMap = getTexture(aiScene, aiMaterial, aiTextureType_DIFFUSE, PixelFormat::RGBA8, &hasAlbedoMap);
		outMaterial->m_EmissiveMap = getTexture(aiScene, aiMaterial, aiTextureType_EMISSIVE, PixelFormat::RGBA8, &hasEmissiveMap);
		outMaterial->m_NormalMap = getTexture(aiScene, aiMaterial, aiTextureType_NORMALS, PixelFormat::RGBA8, &hasNormalMap);
		outMaterial->m_MetallicMap = getTexture(aiScene, aiMaterial, aiTextureType_METALNESS, PixelFormat::RGBA8, &hasMetallicMap);
		outMaterial->m_RoughnessMap = getTexture(aiScene, aiMaterial, aiTextureType_DIFFUSE_ROUGHNESS, PixelFormat::RGBA8, &hasRoughnessMap);
		outMaterial->m_OcclusionMap = getTexture(aiScene, aiMaterial, aiTextureType_AMBIENT_OCCLUSION, PixelFormat::RGBA8, &hasOcclusionMap);

		static const HashSet<String> imageExtensions = {".PNG", ".png", ".jpg", ".JPG", ".jpeg", ".JPEG", ".tga", ".TGA", ".gif", ".GIF", ".bmp", ".BMP"};
		static const HashSet<String> metRoughKeys = {"met", "rough", "Rough", "Met"};
		static const HashSet<String> occlusionKeys = {"occlusion", "AO", "ao"};

		for(const String& file : Files::listFiles(m_Dir)) {

			String filename = Files::getName(file);
			String extension = Files::extension(filename);

			if(imageExtensions.find(extension) == imageExtensions.end()) continue;

			if(std::find(m_LoadedTextureNames.begin(), m_LoadedTextureNames.end(), filename) != m_LoadedTextureNames.end()) {
				continue;
			}

			auto texture = Assets::textures().load(file, PixelFormat::RGBA8);
			texture->setName(filename);

			if(!hasMetallicRoughnessMap && !hasMetallicMap && !hasRoughnessMap && containsAny(filename, metRoughKeys)) {

				outMaterial->m_MetallicRoughnessMap = texture;
				hasMetallicRoughnessMap = true;
				outMaterial->useCombinedMetallicRoughness(hasMetallicRoughnessMap);

			} else if(!hasOcclusionMap && containsAny(filename, occlusionKeys)) {

				outMaterial->m_OcclusionMap = texture;
				hasOcclusionMap = true;
			}
		}
	}
}

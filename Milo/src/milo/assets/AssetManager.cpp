#include "milo/assets/AssetManager.h"

namespace milo {

	MeshManager* AssetManager::s_MeshManager = nullptr;
	TextureManager* AssetManager::s_TextureManager = nullptr;
	MaterialManager* AssetManager::s_MaterialManager = nullptr;
	ModelManager* AssetManager::s_ModelManager = nullptr;
	ShaderManager* AssetManager::s_ShaderManager = nullptr;
	SkyboxManager* AssetManager::s_SkyboxManager = nullptr;

	MeshManager& AssetManager::meshes() {
		return *s_MeshManager;
	}

	TextureManager& AssetManager::textures() {
		return *s_TextureManager;
	}

	MaterialManager& AssetManager::materials() {
		return *s_MaterialManager;
	}

	ModelManager& AssetManager::models() {
		return *s_ModelManager;
	}

	ShaderManager& AssetManager::shaders() {
		return *s_ShaderManager;
	}

	SkyboxManager& AssetManager::skybox() {
		return *s_SkyboxManager;
	}

	void AssetManager::init() {
		s_TextureManager = new TextureManager();
		s_MaterialManager = new MaterialManager();
		s_MeshManager = new MeshManager();
		s_ModelManager = new ModelManager();
		s_ShaderManager = new ShaderManager();
		s_SkyboxManager = new SkyboxManager();

		s_TextureManager->init();
		s_MaterialManager->init();
		s_MeshManager->init();
		s_ModelManager->init();
		s_SkyboxManager->init();
	}

	void AssetManager::shutdown() {
		DELETE_PTR(s_SkyboxManager);
		DELETE_PTR(s_ShaderManager);
		DELETE_PTR(s_MeshManager);
		DELETE_PTR(s_ModelManager);
		DELETE_PTR(s_MaterialManager);
		DELETE_PTR(s_TextureManager);
	}
}
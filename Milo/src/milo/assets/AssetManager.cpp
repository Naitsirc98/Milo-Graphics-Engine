#include "milo/assets/AssetManager.h"

namespace milo {

	MeshManager* AssetManager::s_MeshManager = nullptr;
	MaterialManager* AssetManager::s_MaterialManager = nullptr;
	ShaderManager* AssetManager::s_ShaderManager = nullptr;

	MeshManager& AssetManager::meshes() {
		return *s_MeshManager;
	}

	MaterialManager& AssetManager::materials() {
		return *s_MaterialManager;
	}

	ShaderManager& AssetManager::shaders() {
		return *s_ShaderManager;
	}

	void AssetManager::init() {
		s_MeshManager = new MeshManager();
		s_MaterialManager = new MaterialManager();
		s_ShaderManager = new ShaderManager();
	}

	void AssetManager::shutdown() {
		DELETE_PTR(s_ShaderManager);
		DELETE_PTR(s_MaterialManager);
		DELETE_PTR(s_MeshManager);
	}
}
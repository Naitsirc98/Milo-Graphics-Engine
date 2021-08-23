#include "milo/assets/AssetManager.h"

namespace milo {

	MeshManager* AssetManager::s_MeshManager = nullptr;
	MaterialManager* AssetManager::s_MaterialManager = nullptr;

	MeshManager& AssetManager::meshes() {
		return *s_MeshManager;
	}

	MaterialManager& AssetManager::materials() {
		return *s_MaterialManager;
	}

	void AssetManager::init() {
		s_MeshManager = new MeshManager();
		s_MaterialManager = new MaterialManager();
	}

	void AssetManager::shutdown() {
		DELETE_PTR(s_MaterialManager);
		DELETE_PTR(s_MeshManager);
	}
}
#pragma once

#include "milo/assets/meshes/MeshManager.h"
#include "milo/assets/materials/MaterialManager.h"

namespace milo {

	class AssetManager {
		friend class MiloSubSystemManager;
	private:
		static MeshManager* s_MeshManager;
		static MaterialManager* s_MaterialManager;
		// TODO ...
	public:
		static MeshManager& meshes();
		static MaterialManager& materials();
	private:
		static void init();
		static void shutdown();
	};

	using Assets = AssetManager;
}
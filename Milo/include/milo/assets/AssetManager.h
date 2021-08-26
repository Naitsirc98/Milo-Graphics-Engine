#pragma once

#include "milo/assets/meshes/MeshManager.h"
#include "milo/assets/materials/MaterialManager.h"
#include "milo/assets/shaders/ShaderManager.h"

namespace milo {

	class AssetManager {
		friend class MiloSubSystemManager;
	private:
		static MeshManager* s_MeshManager;
		static MaterialManager* s_MaterialManager;
		static ShaderManager* s_ShaderManager;
		// TODO ...
	public:
		static MeshManager& meshes();
		static MaterialManager& materials();
		static ShaderManager& shaders();
	private:
		static void init();
		static void shutdown();
	};

	using Assets = AssetManager;
}
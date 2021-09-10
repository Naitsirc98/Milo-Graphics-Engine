#pragma once

#include "milo/assets/meshes/MeshManager.h"
#include "milo/assets/textures/TextureManager.h"
#include "milo/assets/materials/MaterialManager.h"
#include "milo/assets/shaders/ShaderManager.h"
#include "milo/assets/skybox/SkyboxManager.h"
#include "milo/assets/models/ModelManager.h"

namespace milo {

	class AssetManager {
		friend class MiloSubSystemManager;
	private:
		static MeshManager* s_MeshManager;
		static TextureManager* s_TextureManager;
		static MaterialManager* s_MaterialManager;
		static ModelManager* s_ModelManager;
		static ShaderManager* s_ShaderManager;
		static SkyboxManager* s_SkyboxManager;
		// TODO ...
	public:
		static MeshManager& meshes();
		static TextureManager& textures();
		static MaterialManager& materials();
		static ModelManager& models();
		static ShaderManager& shaders();
		static SkyboxManager& skybox();
	private:
		static void init();
		static void shutdown();
	};

	using Assets = AssetManager;
}
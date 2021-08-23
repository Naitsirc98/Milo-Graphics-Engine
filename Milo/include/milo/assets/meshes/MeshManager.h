#pragma once

#include "Mesh.h"

namespace milo {

	class MeshLoader;

	class MeshManager {
		friend class AssetManager;
	private:
		HashMap<String, Mesh*> m_Meshes;
		Mutex m_Mutex;
	private:
		MeshManager() = default;
		~MeshManager();
	public:
		Mesh* load(const String& filename);
		bool exists(const String& filename);
		Mesh* find(const String& filename);
		void destroy(const String& filename);
	private:
		static Shared<MeshLoader> getMeshLoaderOf(const String& filename);
		static void createGraphicsBuffers(const String& filename, Mesh* mesh);
		static void createBoundingVolume(const String& filename, Mesh* mesh);
	};

}
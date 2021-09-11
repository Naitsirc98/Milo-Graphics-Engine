#pragma once

#include "Mesh.h"

namespace milo {

	class MeshLoader;

	class MeshManager {
		friend class AssetManager;
		friend class AssimpModelLoader;
	private:
		HashMap<String, Mesh*> m_Meshes;
		Mutex m_Mutex;
	private:
		MeshManager();
		~MeshManager();
		void init();
	public:
		Mesh* getQuad() const;
		Mesh* getPlane() const;
		Mesh* getCube() const;
		Mesh* getSphere() const;
		Mesh* getCylinder() const;
		Mesh* getMonkey() const;
		Mesh* load(const String& name, const String& filename);
		bool exists(const String& name);
		Mesh* find(const String& name);
		void destroy(const String& name);
	private:
		void addMesh(const String& name, Mesh* mesh);
	private:
		static Ref<MeshLoader> getMeshLoaderOf(const String& filename);
		static void createGraphicsBuffers(const String& filename, Mesh* mesh);
		static void createBoundingVolume(const String& filename, Mesh* mesh);
	};

}
#include "milo/assets/meshes/MeshManager.h"
#include "milo/assets/meshes/MeshLoader.h"
#include "milo/io/Files.h"
#include "milo/assets/meshes/loaders/ObjMeshLoader.h"

namespace milo {

	MeshManager::~MeshManager() {
		for(auto& [filename, mesh] : m_Meshes) {
			DELETE_PTR(mesh);
		}
		m_Meshes.clear();
	}

	Mesh* MeshManager::load(const String& filename) {
		Mesh* mesh = nullptr;
		m_Mutex.lock();
		{
			if(exists(filename)) {
				mesh = find(filename);
			} else {
				Shared<MeshLoader> loader = getMeshLoaderOf(filename);
				if(loader) {
					mesh = loader->load(filename);
					createGraphicsBuffers(filename, mesh);
					createBoundingVolume(filename, mesh);
					m_Meshes[filename] = mesh;
				}
			}
		}
		m_Mutex.unlock();
		return mesh;
	}

	bool MeshManager::exists(const String& filename) {
		return m_Meshes.find(filename) != m_Meshes.end();
	}

	Mesh* MeshManager::find(const String& filename) {
		return exists(filename) ? m_Meshes[filename] : nullptr;
	}

	void MeshManager::destroy(const String& filename) {
		if(!exists(filename)) return;
		m_Mutex.lock();
		{
			Mesh* mesh = m_Meshes[filename];
			DELETE_PTR(mesh);
			m_Meshes.erase(filename);
		}
		m_Mutex.unlock();
	}

	Shared<MeshLoader> MeshManager::getMeshLoaderOf(const String& filename) {

		switch(MeshFormats::formatOf(filename)) {
			case MeshFormat::Obj:
				return std::make_shared<ObjMeshLoader>();
			case MeshFormat::Unknown:
			default:
				Log::error("Mesh format {} unsupported", Files::extension(filename));
				return nullptr;
		}
	}

	void MeshManager::createGraphicsBuffers(const String& filename, Mesh* mesh) {
		mesh->m_Buffers = Mesh::GraphicsBuffers::create(mesh->vertices(), mesh->indices());
	}

	void MeshManager::createBoundingVolume(const String& filename, Mesh* mesh) {
		// TODO
	}
}
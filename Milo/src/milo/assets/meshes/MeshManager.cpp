#include "milo/assets/meshes/MeshManager.h"
#include "milo/assets/meshes/MeshLoader.h"
#include "milo/io/Files.h"
#include "milo/assets/meshes/loaders/ObjMeshLoader.h"
#include "milo/assets/meshes/loaders/AssimpLoader.h"
#include "milo/assets/AssetManager.h"

#define CUBE_MESH_NAME "SM_Cube"
#define SPHERE_MESH_NAME "SM_Sphere"
#define PLANE_MESH_NAME "SM_Plane"
#define QUAD_MESH_NAME "SM_Quad"
#define CYLINDER_MESH_NAME "SM_Cylinder"
#define MONKEY_MESH_NAME "SM_Monkey"
#define SPONZA_MESH_NAME "SM_Sponza"

namespace milo {

	MeshManager::MeshManager() {
	}

	MeshManager::~MeshManager() {
		for(auto& [filename, mesh] : m_Meshes) {
			DELETE_PTR(mesh);
		}
		m_Meshes.clear();
	}

	void MeshManager::init() {
		load(CUBE_MESH_NAME, "resources/meshes/Cube.obj");
		load(SPHERE_MESH_NAME, "resources/meshes/Sphere.obj");
		load(PLANE_MESH_NAME, "resources/meshes/Plane.obj");
		load(QUAD_MESH_NAME, "resources/meshes/Quad.obj");
		load(CYLINDER_MESH_NAME, "resources/meshes/Cylinder.obj");
		load(MONKEY_MESH_NAME, "resources/meshes/Monkey.obj");
		load(SPONZA_MESH_NAME, "resources/meshes/Sponza/Sponza.obj");
	}

	Mesh* MeshManager::getQuad() const {
		return m_Meshes.at(QUAD_MESH_NAME);
	}

	Mesh* MeshManager::getPlane() const {
		return m_Meshes.at(PLANE_MESH_NAME);
	}

	Mesh* MeshManager::getCube() const {
		return m_Meshes.at(CUBE_MESH_NAME);
	}

	Mesh* MeshManager::getSphere() const {
		return m_Meshes.at(SPHERE_MESH_NAME);
	}

	Mesh* MeshManager::getCylinder() const {
		return m_Meshes.at(CYLINDER_MESH_NAME);
	}

	Mesh* MeshManager::getMonkey() const {
		return m_Meshes.at(MONKEY_MESH_NAME);
	}

	Mesh* MeshManager::getSponza() const {
		return m_Meshes.at(SPONZA_MESH_NAME);
	}

	Mesh* MeshManager::load(const String& name, const String& filename) {
		Mesh* mesh = nullptr;
		m_Mutex.lock();
		{
			if(exists(name)) {
				mesh = find(name);
			} else {
				Ref<MeshLoader> loader = getMeshLoaderOf(filename);
				if(loader) {
					mesh = loader->load(filename);
					createGraphicsBuffers(filename, mesh);
					createBoundingVolume(filename, mesh);
					mesh->m_Icon = Assets::textures().createIcon(name, mesh, Assets::materials().getDefault());
					m_Meshes[name] = mesh;
				}
			}
		}
		m_Mutex.unlock();
		return mesh;
	}

	bool MeshManager::exists(const String& name) {
		return m_Meshes.find(name) != m_Meshes.end();
	}

	Mesh* MeshManager::find(const String& name) {
		return exists(name) ? m_Meshes[name] : nullptr;
	}

	void MeshManager::destroy(const String& name) {
		if(!exists(name)) return;
		m_Mutex.lock();
		{
			Mesh* mesh = m_Meshes[name];
			DELETE_PTR(mesh);
			m_Meshes.erase(name);
		}
		m_Mutex.unlock();
	}

	Ref<MeshLoader> MeshManager::getMeshLoaderOf(const String& filename) {

		switch(MeshFormats::formatOf(filename)) {
			case MeshFormat::Obj:
				return std::make_shared<ObjMeshLoader>();
			//case MeshFormat::Unknown:
			default:
				return std::make_shared<AssimpLoader>();
				//return nullptr;
		}
	}

	void MeshManager::createGraphicsBuffers(const String& filename, Mesh* mesh) {
		mesh->m_Buffers = Mesh::GraphicsBuffers::create(mesh->vertices(), mesh->indices());
	}

	void MeshManager::createBoundingVolume(const String& filename, Mesh* mesh) {
		// TODO
	}
}
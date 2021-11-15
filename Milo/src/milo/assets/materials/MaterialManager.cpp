#include "milo/assets/materials/MaterialManager.h"
#include "milo/io/Files.h"
#define JSON_IMPLEMENTATION
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <json.hpp>
#include "milo/assets/AssetManager.h"

#define DEFAULT_MATERIAL_NAME "M_DefaultMaterial"

namespace milo {

	MaterialManager::MaterialManager() {
	}

	MaterialManager::~MaterialManager() {

		for(auto& [name, material] : m_Materials) {
			DELETE_PTR(material);
		}
		m_Materials.clear();

		DELETE_PTR(m_ResourcePool);
	}

	void MaterialManager::init() {
		m_ResourcePool = MaterialResourcePool::create();
		load(DEFAULT_MATERIAL_NAME, "resources/materials/M_DefaultMaterial.mat");
	}

	Material* MaterialManager::getDefault() const {
		return m_Materials.at(DEFAULT_MATERIAL_NAME);
	}

	Material* MaterialManager::create(const String& name, const String& filename) {
		if(exists(name)) return find(name);

		String file = filename;

		if(file == "") {
			file = str("resources/materials/") + name + ".mat";
		}

		Material* material = new Material(name, file);

		m_Materials[name] = material;
		m_ResourcePool->allocateMaterialResources(material);
		if(name == DEFAULT_MATERIAL_NAME) {
			material->m_Icon = Assets::textures().getIcon("DefaultMaterialIcon");
		} else {
			material->m_Icon = Assets::textures().createIcon(name, Assets::meshes().getSphere(), material);
		}

		return material;
	}

	Material* MaterialManager::load(const String& name, const String& filename, bool replace) {
		Material* material = nullptr;
		m_Mutex.lock();
		{
			if(exists(name)) {
				material = find(name);
				if(replace) {
					load(name, filename, material);
				}
			} else {
				if(load(name, filename, material)) {
					m_Materials[name] = material;
					m_ResourcePool->allocateMaterialResources(material);
					if(name == DEFAULT_MATERIAL_NAME) {
						material->m_Icon = Assets::textures().getIcon("DefaultMaterialIcon");
					} else {
						material->m_Icon = Assets::textures().createIcon(name, Assets::meshes().getSphere(), material);
					}
				}
			}
		}
		m_Mutex.unlock();
		return material;
	}

	bool MaterialManager::exists(const String& name) {
		return m_Materials.find(name) != m_Materials.end();
	}

	Material* MaterialManager::find(const String& name) {
		return exists(name) ? m_Materials[name] : nullptr;
	}

	void MaterialManager::destroy(const String& name) {
		if(!exists(name)) return;
		m_Mutex.lock();
		{
			Material* material = m_Materials[name];
			DELETE_PTR(material);
			m_Materials.erase(name);
			m_ResourcePool->freeMaterialResources(material);
		}
		m_Mutex.unlock();
	}

	void MaterialManager::addMaterial(const String& name, Material* material) {
		if(exists(name)) {
			destroy(name);
		}
		m_Materials[name] = material;
		m_ResourcePool->allocateMaterialResources(material);
		material->m_Icon = Assets::textures().createIcon(name, Assets::meshes().getSphere(), material);
	}

	MaterialResourcePool& MaterialManager::resourcePool() const {
		return *m_ResourcePool;
	}

	bool MaterialManager::load(const String& name, const String& filename, Material*& material) {
		if(!Files::exists(filename)) return false;
		if(Files::isDirectory(filename)) return false;

		material = new Material(name, filename);

		try {
			nlohmann::json json;
			{
				InputStream inputStream(filename);
				inputStream >> json;
			}

			if(json.contains("albedo")) {
				float color[4];
				json["albedo"].get_to(color);
				material->m_Data.albedo = {color[0], color[1], color[2], color[3]};
			}

			if(json.contains("metallic")) {
				material->m_Data.metallic = json["metallic"].get<float>();
			}

			if(json.contains("roughness")) {
				material->m_Data.roughness = json["roughness"].get<float>();
			}

			if(json.contains("emissive")) {
				float color[4];
				json["emissive"].get_to(color);
				material->m_Data.emissiveColor = {color[0], color[1], color[2], color[3]};
			}

			material->m_AlbedoMap = loadTexture2D(&json, "albedoMap", filename);
			material->m_NormalMap = loadTexture2D(&json, "normalMap", filename);
			material->m_MetallicMap = loadTexture2D(&json, "metallicMap", filename);
			material->m_RoughnessMap = loadTexture2D(&json, "roughnessMap", filename);
			material->m_OcclusionMap = loadTexture2D(&json, "occlusionMap", filename);

			material->useNormalMap(material->m_NormalMap != nullptr);
		} catch(...) {
			Log::error("Failed to parse material {}", material->name());
			return false;
		}

		return true;
	}

	Ref<Texture2D> MaterialManager::loadTexture2D(void* pJson, const String& textureName, const String& materialFile) {

		nlohmann::json& json = *(nlohmann::json*)pJson;

		Ref<Texture2D> texture = Assets::textures().whiteTexture();

		if(json.contains(textureName)) {

			String texturePath = json[textureName].get<String>();
			if(!Files::isAbsolute(texturePath)) {
				texturePath = Files::append(Files::parentOf(materialFile), texturePath);
			}

			texture = Assets::textures().load(texturePath, PixelFormat::RGBA8);
			texture->generateMipmaps();
		}

		return texture;
	}

	void MaterialManager::update() {
		for(auto& [name, material] : m_Materials) {
			if(material->dirty()) {
				m_ResourcePool->updateMaterial(material);
				material->m_Dirty = false;
				material->m_Version++;
			}
		}
	}
}
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

	Material* MaterialManager::load(const String& name, const String& filename) {
		Material* material = nullptr;
		m_Mutex.lock();
		{
			if(exists(name)) {
				material = find(name);
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
	}

	MaterialResourcePool& MaterialManager::resourcePool() const {
		return *m_ResourcePool;
	}

	bool MaterialManager::load(const String& name, const String& filename, Material*& material) {
		if(!Files::exists(filename)) return false;
		if(Files::isDirectory(filename)) return false;

		material = new Material(name, filename);

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

		material->m_AlbedoMap = loadTexture2D(&json, "albedoMap", filename);

		// TODO

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

			if(Files::exists(texturePath)) {

				Image* image = Image::loadImage(texturePath, PixelFormat::SRGBA);
				texture = Ref<Texture2D>(Texture2D::create());

				Texture2D::AllocInfo allocInfo = {};
				allocInfo.width = image->width();
				allocInfo.height = image->height();
				allocInfo.format = image->format();
				allocInfo.pixels = image->pixels();

				texture->allocate(allocInfo);
				texture->generateMipmaps();

				DELETE_PTR(image);
			}
		}

		return texture;
	}

}
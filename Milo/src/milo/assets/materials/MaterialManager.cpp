#include "milo/assets/materials/MaterialManager.h"
#include "milo/io/Files.h"
#define JSON_IMPLEMENTATION
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>

#define DEFAULT_MATERIAL_NAME "M_DefaultMaterial"

namespace milo {

	MaterialManager::MaterialManager() {
		load(DEFAULT_MATERIAL_NAME, "resources/materials/M_DefaultMaterial.mat");
	}

	MaterialManager::~MaterialManager() {
		for(auto& [name, material] : m_Materials) {
			DELETE_PTR(material);
		}
		m_Materials.clear();
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
					m_Materials[filename] = material;
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
		}
		m_Mutex.unlock();
	}

	bool MaterialManager::load(const String& name, const String& filename, Material*& material) {
		if(!Files::exists(filename)) return false;
		if(Files::isDirectory(filename)) return false;

		material = new Material(name);

		nlohmann::json json;
		{
			InputStream inputStream(filename);
			inputStream >> json;
		}

		if(json.contains("baseColor")) {
			float color[4];
			json["baseColor"].get_to(color);
			material->m_BaseColor = {color[0], color[1], color[2], color[3]};
		}

		if(json.contains("baseColorTexture")) {
			String texturePath = json["baseColorTexture"].get<String>();
			if(!Files::isAbsolute(texturePath)) {
				texturePath = Files::append(Files::parentOf(filename), texturePath);
			}
			if(Files::exists(texturePath)) {
				Image* image = Image::loadImage(texturePath, PixelFormat::RGBA8);
				Texture2D* texture = Texture2D::create();

				Texture2D::AllocInfo allocInfo = {};
				allocInfo.width = image->width();
				allocInfo.height = image->height();
				allocInfo.format = image->format();
				allocInfo.pixels = image->pixels();

				texture->allocate(allocInfo);
				texture->generateMipmaps();

				material->m_BaseColorTexture = texture;

				DELETE_PTR(image);
			}
		}

		// TODO

		return true;
	}
}
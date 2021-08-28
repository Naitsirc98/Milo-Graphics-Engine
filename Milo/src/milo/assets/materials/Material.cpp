#include "milo/assets/materials/Material.h"

namespace milo {

	Material::Material(String name, String filename) : m_Name(std::move(name)), m_Filename(std::move(filename)) {
	}

	Material::~Material() {
		// TODO
		//DELETE_PTR(m_AlbedoMap);
	}

	const String& Material::name() const {
		return m_Name;
	}

	const String& Material::filename() const {
		return m_Filename;
	}

	const Color& Material::albedo() const {
		return m_Data.albedo;
	}

	Texture2D* Material::albedoMap() const {
		return m_AlbedoMap;
	}

	const Material::Data& Material::data() const {
		return m_Data;
	}
}
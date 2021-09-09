#include "milo/assets/materials/Material.h"

namespace milo {

	Material::Material(String name, String filename) : Asset(name, filename) {
	}

	Material::~Material() = default;

	const Color& Material::albedo() const {
		return m_Data.albedo;
	}

	Ref<Texture2D> Material::albedoMap() const {
		return m_AlbedoMap;
	}

	const Material::Data& Material::data() const {
		return m_Data;
	}
}
#include "milo/assets/materials/Material.h"

namespace milo {

	Material::Material(String filename) : m_Filename(std::move(filename)) {
	}

	Material::~Material() {
		DELETE_PTR(m_BaseColorTexture);
	}

	const String& Material::filename() const {
		return m_Filename;
	}

	const Color& Material::baseColor() const {
		return m_BaseColor;
	}

	Texture2D* Material::baseColorTexture() const {
		return m_BaseColorTexture;
	}
}
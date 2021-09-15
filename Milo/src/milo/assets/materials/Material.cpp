#include "milo/assets/materials/Material.h"

namespace milo {

	Material::Material(String name, String filename) : Asset(name, filename) {
	}

	Material::~Material() = default;

	const Color& Material::albedo() const {
		return m_Data.albedo;
	}

	Material* Material::albedo(const Color& color) {
		m_Data.albedo = color;
		return this;
	}

	const Color& Material::emissiveColor() const {
		return m_Data.emissiveColor;
	}

	Material* Material::emissiveColor(const Color& color) {
		m_Data.emissiveColor = color;
		return this;
	}

	float Material::alpha() const {
		return m_Data.alpha;
	}

	Material* Material::alpha(float value) {
		m_Data.alpha = value;
		return this;
	}

	float Material::metallic() const {
		return m_Data.metallic;
	}

	Material* Material::metallic(float value) {
		m_Data.metallic = value;
		return this;
	}

	float Material::roughness() const {
		return m_Data.roughness;
	}

	Material* Material::roughness(float value) {
		m_Data.roughness = value;
		return this;
	}

	float Material::occlusion() const {
		return m_Data.occlusion;
	}

	Material* Material::occlusion(float value) {
		m_Data.occlusion = value;
		return this;
	}

	float Material::fresnel0() const {
		return m_Data.fresnel0;
	}

	Material* Material::fresnel0(float value) {
		m_Data.fresnel0 = value;
		return this;
	}

	float Material::normalScale() const {
		return m_Data.normalScale;
	}

	Material* Material::normalScale(float value) {
		m_Data.normalScale = value;
		return this;
	}

	bool Material::useNormalMap() const {
		return m_Data.useNormalMap;
	}

	Material* Material::useNormalMap(bool value) {
		m_Data.useNormalMap = value;
		return this;
	}

	bool Material::useCombinedMetallicRoughnessMap() const {
		return m_Data.useCombinedMetallicRoughnessMap;
	}

	Material* Material::useCombinedMetallicRoughnessMap(bool value) {
		m_Data.useCombinedMetallicRoughnessMap = value;
		return this;
	}

	Ref<Texture2D> Material::albedoMap() const {
		return m_AlbedoMap;
	}

	Material* Material::albedoMap(Ref<Texture2D> texture) {
		m_AlbedoMap = texture;
		return this;
	}

	Ref<Texture2D> Material::emissiveMap() const {
		return m_EmissiveMap;
	}

	Material* Material::emissiveMap(Ref<Texture2D> texture) {
		m_EmissiveMap = texture;
		return this;
	}

	Ref<Texture2D> Material::normalMap() const {
		return m_NormalMap;
	}

	Material* Material::normalMap(Ref<Texture2D> texture) {
		m_NormalMap = texture;
		return this;
	}

	Ref<Texture2D> Material::metallicMap() const {
		return m_MetallicMap;
	}

	Material* Material::metallicMap(Ref<Texture2D> texture) {
		m_MetallicMap = texture;
		return this;
	}

	Ref<Texture2D> Material::occlusionMap() const {
		return m_OcclusionMap;
	}

	Material* Material::occlusionMap(Ref<Texture2D> texture) {
		m_OcclusionMap = texture;
		return this;
	}

	Ref<Texture2D> Material::roughnessMap() const {
		return m_RoughnessMap;
	}

	Material* Material::roughnessMap(Ref<Texture2D> texture) {
		m_RoughnessMap = texture;
		return this;
	}

	const Material::Data& Material::data() const {
		return m_Data;
	}
}
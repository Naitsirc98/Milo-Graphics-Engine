#include "milo/assets/materials/Material.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	Material::Material(String name, String filename) : Asset(name, filename) {
		m_AlbedoMap = Assets::textures().whiteTexture();
		m_EmissiveMap = Assets::textures().blackTexture();
		m_NormalMap = Assets::textures().whiteTexture();
		m_MetallicMap = Assets::textures().whiteTexture();
		m_RoughnessMap = Assets::textures().whiteTexture();
		m_MetallicRoughnessMap = Assets::textures().whiteTexture();
		m_OcclusionMap = Assets::textures().whiteTexture();
	}

	Material::~Material() = default;

	const Color& Material::albedo() const {
		return m_Data.albedo;
	}

	Material* Material::albedo(const Color& color) {
		m_Data.albedo = color;
		m_Dirty = true;
		return this;
	}

	const Color& Material::emissiveColor() const {
		return m_Data.emissiveColor;
	}

	Material* Material::emissiveColor(const Color& color) {
		m_Data.emissiveColor = color;
		m_Dirty = true;
		return this;
	}

	float Material::alpha() const {
		return m_Data.alpha;
	}

	Material* Material::alpha(float value) {
		m_Data.alpha = value;
		m_Dirty = true;
		return this;
	}

	float Material::metallic() const {
		return m_Data.metallic;
	}

	Material* Material::metallic(float value) {
		m_Data.metallic = value;
		m_Dirty = true;
		return this;
	}

	float Material::roughness() const {
		return m_Data.roughness;
	}

	Material* Material::roughness(float value) {
		m_Data.roughness = value;
		m_Dirty = true;
		return this;
	}

	float Material::occlusion() const {
		return m_Data.occlusion;
	}

	Material* Material::occlusion(float value) {
		m_Data.occlusion = value;
		m_Dirty = true;
		return this;
	}

	float Material::fresnel0() const {
		return m_Data.fresnel0;
	}

	Material* Material::fresnel0(float value) {
		m_Data.fresnel0 = value;
		m_Dirty = true;
		return this;
	}

	float Material::normalScale() const {
		return m_Data.normalScale;
	}

	Material* Material::normalScale(float value) {
		m_Data.normalScale = value;
		m_Dirty = true;
		return this;
	}

	bool Material::useNormalMap() const {
		return m_Data.useNormalMap;
	}

	Material* Material::useNormalMap(bool value) {
		m_Data.useNormalMap = value;
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::albedoMap() const {
		return m_AlbedoMap;
	}

	Material* Material::albedoMap(Ref<Texture2D> texture) {
		m_AlbedoMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::emissiveMap() const {
		return m_EmissiveMap;
	}

	Material* Material::emissiveMap(Ref<Texture2D> texture) {
		m_EmissiveMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::normalMap() const {
		return m_NormalMap;
	}

	Material* Material::normalMap(Ref<Texture2D> texture) {
		m_NormalMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::metallicMap() const {
		return m_MetallicMap;
	}

	Material* Material::metallicMap(Ref<Texture2D> texture) {
		m_MetallicMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::occlusionMap() const {
		return m_OcclusionMap;
	}

	Material* Material::occlusionMap(Ref<Texture2D> texture) {
		m_OcclusionMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	Ref<Texture2D> Material::roughnessMap() const {
		return m_RoughnessMap;
	}

	Material* Material::roughnessMap(Ref<Texture2D> texture) {
		m_RoughnessMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	bool Material::useCombinedMetallicRoughness() const {
		return m_Data.useCombinedMetallicRoughness;
	}

	void Material::useCombinedMetallicRoughness(bool value) {
		m_Data.useCombinedMetallicRoughness = value;
	}

	Ref<Texture2D> Material::metallicRoughnessMap() const {
		return m_MetallicRoughnessMap;
	}

	Material* Material::metallicRoughnessMap(Ref<Texture2D> texture) {
		m_MetallicRoughnessMap = std::move(texture);
		m_Dirty = true;
		return this;
	}

	const Material::Data& Material::data() const {
		return m_Data;
	}

	bool Material::dirty() const {
		return m_Dirty;
	}

	uint32_t Material::version() const {
		return m_Version;
	}
}
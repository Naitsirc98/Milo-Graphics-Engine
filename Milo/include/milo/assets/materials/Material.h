#pragma once

#include "milo/assets/Asset.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Material : public Asset {
		friend class MaterialManager;
		friend class MaterialResourcePool;
		friend class AssimpModelLoader;
	public:
		struct Data {
			Color albedo{Colors::WHITE};
			Color emissiveColor{Colors::BLACK};
			// Values
			float alpha{1.0f};
			float metallic{1.0f};
			float roughness{1.0f};
			float occlusion{1.0f};
			float fresnel0{0.02f};
			float normalScale{1.0f};
			// Flags
			bool useNormalMap{false};
			bool useCombinedMetallicRoughnessMap{true};
		};
	public:
		inline static const uint32_t TEXTURE_COUNT = 1;
	private:
		Material::Data m_Data{};
		// Textures
		Ref<Texture2D> m_AlbedoMap{nullptr};
		Ref<Texture2D> m_MetallicMap{nullptr};
		Ref<Texture2D> m_RoughnessMap{nullptr};
		Ref<Texture2D> m_MetallicRoughnessMap{nullptr};
		Ref<Texture2D> m_OcclusionMap{nullptr};
		Ref<Texture2D> m_EmissiveMap{nullptr};
		Ref<Texture2D> m_NormalMap{nullptr};
	private:
		explicit Material(String name, String filename);
		~Material();
	public:
		const Color& albedo() const;
		Ref<Texture2D> albedoMap() const;
		const Material::Data& data() const;
	};
}
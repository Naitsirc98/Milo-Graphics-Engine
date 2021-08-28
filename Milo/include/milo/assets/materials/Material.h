#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Material {
		friend class MaterialManager;
		friend class MaterialResourcePool;
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
		String m_Name;
		String m_Filename;
		Material::Data m_Data{};
		// Textures
		Texture2D* m_AlbedoMap{nullptr};
		Texture2D* m_MetallicMap{nullptr};
		Texture2D* m_RoughnessMap{nullptr};
		Texture2D* m_MetallicRoughnessMap{nullptr};
		Texture2D* m_OcclusionMap{nullptr};
		Texture2D* m_EmissiveMap{nullptr};
		Texture2D* m_NormalMap{nullptr};
		MaterialResourcePool* m_ResourcePool{nullptr};
	private:
		explicit Material(String name, String filename);
		~Material();
	public:
		const String& name() const;
		const String& filename() const;
		const Color& albedo() const;
		Texture2D* albedoMap() const;
		const Material::Data& data() const;

		template<typename T>
		inline T* resourcePool() const {
			return (T*) m_ResourcePool;
		}
	};
}
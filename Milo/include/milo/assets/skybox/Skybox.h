#pragma once

#include "milo/assets/Asset.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Skybox : public Asset {
		friend class SkyboxManager;
		friend class SkyboxFactory;
		friend class VulkanSkyboxFactory;
	protected:
		Ref<Texture2D> m_EquirectangularTexture;
		Cubemap* m_EnvironmentMap{nullptr};
		Cubemap* m_IrradianceMap{nullptr};
		Cubemap* m_PrefilterMap{nullptr};
		Texture2D* m_BRDFMap{nullptr};
		float m_MaxPrefilterLOD{4.0f};
		float m_PrefilterLODBias{0.2f};
		uint32_t m_Modifications{1};
	protected:
		Skybox(String name, String filename);
		virtual ~Skybox();
	public:
		Ref<Texture2D> equirectangularTexture() const;
		Cubemap* environmentMap() const;
		Cubemap* irradianceMap() const;
		Cubemap* prefilterMap() const;
		Texture2D* brdfMap() const;
		float maxPrefilterLOD() const;
		void maxPrefilterLOD(float value);
		float prefilterLODBias() const;
		void prefilterLODBias(float value);
		uint32_t modifications() const;
	};

	class PreethamSky : public Skybox {
		friend class SkyboxManager;
		friend class SkyboxFactory;
		friend class VulkanSkyboxFactory;
	private:
		float m_Turbidity{2};
		float m_Azimuth{0};
		float m_Inclination{0};
		bool m_Dirty{true};
	private:
		PreethamSky(const String& name);
		~PreethamSky() override;
	public:
		float turbidity() const;
		PreethamSky* turbidity(float value);
		float azimuth() const;
		PreethamSky* azimuth(float value);
		float inclination() const;
		PreethamSky* inclination(float value);
		bool dirty() const;
		void update();
	};
}
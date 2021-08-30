#pragma once

#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Skybox {
		friend class SkyboxManager;
		friend class SkyboxFactory;
		friend class VulkanSkyboxFactory;
	private:
		String m_Name;
		String m_Filename;
		Cubemap* m_EnvironmentMap{nullptr};
		Cubemap* m_IrradianceMap{nullptr};
		Cubemap* m_PrefilterMap{nullptr};
		Texture2D* m_BRDFMap{nullptr};
		float m_MaxPrefilterLOD{4.0f};
		float m_PrefilterLODBias{-0.25f};
	private:
		Skybox(String name, String filename);
		~Skybox();
	public:
		const String& name() const;
		const String& filename() const;
		Cubemap* environmentMap() const;
		Cubemap* irradianceMap() const;
		Cubemap* prefilterMap() const;
		Texture2D* brdfMap() const;
		float maxPrefilterLOD() const;
		void maxPrefilterLOD(float value);
		float prefilterLODBias() const;
		void prefilterLODBias(float value);
	};

}
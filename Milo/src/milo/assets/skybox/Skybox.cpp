#include "milo/assets/skybox/Skybox.h"

namespace milo {

	Skybox::Skybox(String name, String filename) : m_Name(std::move(name)), m_Filename(std::move(filename)) {
	}

	Skybox::~Skybox() {
		DELETE_PTR(m_EnvironmentMap);
		DELETE_PTR(m_PrefilterMap);
		DELETE_PTR(m_IrradianceMap);
		DELETE_PTR(m_BRDFMap);
	}

	const String& Skybox::name() const {
		return m_Name;
	}

	const String& Skybox::filename() const {
		return m_Filename;
	}

	Cubemap* Skybox::environmentMap() const {
		return m_EnvironmentMap;
	}

	Cubemap* Skybox::irradianceMap() const {
		return m_IrradianceMap;
	}

	Cubemap* Skybox::prefilterMap() const {
		return m_PrefilterMap;
	}

	Texture2D* Skybox::brdfMap() const {
		return m_BRDFMap;
	}

	float Skybox::maxPrefilterLOD() const {
		return m_MaxPrefilterLOD;
	}

	void Skybox::maxPrefilterLOD(float value) {
		m_MaxPrefilterLOD = value;
	}

	float Skybox::prefilterLODBias() const {
		return m_PrefilterLODBias;
	}

	void Skybox::prefilterLODBias(float value) {
		m_PrefilterLODBias = value;
	}
}
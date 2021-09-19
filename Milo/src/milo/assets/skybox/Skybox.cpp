#include "milo/assets/skybox/Skybox.h"
#include "milo/assets/AssetManager.h"

namespace milo {

	Skybox::Skybox(String name, String filename) : Asset((std::move(name)), std::move(filename)) {

	}

	Skybox::~Skybox() {
		Assets::textures().removeIcon(name());
		DELETE_PTR(m_EnvironmentMap);
		DELETE_PTR(m_PrefilterMap);
		DELETE_PTR(m_IrradianceMap);
		DELETE_PTR(m_BRDFMap);
	}

	Ref<Texture2D> Skybox::equirectangularTexture() const {
		return m_EquirectangularTexture;
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
		++m_Modifications;
	}

	float Skybox::prefilterLODBias() const {
		return m_PrefilterLODBias;
	}

	void Skybox::prefilterLODBias(float value) {
		m_PrefilterLODBias = value;
		++m_Modifications;
	}

	uint32_t Skybox::modifications() const {
		return m_Modifications;
	}

	// =======================

	PreethamSky::PreethamSky(const String& name) : Skybox(name, "") {

	}

	PreethamSky::~PreethamSky() {

	}

	float PreethamSky::turbidity() const {
		return m_Turbidity;
	}

	PreethamSky* PreethamSky::turbidity(float value) {
		if(m_Turbidity == value) return this;
		m_Turbidity = value;
		m_Dirty = true;
		return this;
	}

	float PreethamSky::azimuth() const {
		return m_Azimuth;
	}

	PreethamSky* PreethamSky::azimuth(float value) {
		if(m_Azimuth == value) return this;
		m_Azimuth = value;
		m_Dirty = true;
		return this;
	}

	float PreethamSky::inclination() const {
		return m_Inclination;
	}

	PreethamSky* PreethamSky::inclination(float value) {
		if(m_Inclination == value) return this;
		m_Inclination = value;
		m_Dirty = true;
		return this;
	}

	bool PreethamSky::dirty() const {
		return m_Dirty;
	}

	void PreethamSky::update() {
		Assets::skybox().updatePreethamSky(this);
		++m_Modifications;
		m_Dirty = false;
	}
}
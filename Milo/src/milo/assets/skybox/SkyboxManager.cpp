#include "milo/assets/skybox/SkyboxManager.h"
#include "milo/io/Files.h"

namespace milo {

	static const String PREETHAM_SKYBOX_NAME = "SK_PreethamSky";
	static const String INDOOR_SKYBOX_NAME = "SK_Indoor";

	SkyboxManager::SkyboxManager() {
		m_SkyboxFactory = SkyboxFactory::create();
		createPreethamSky();
		load(INDOOR_SKYBOX_NAME, Files::resource("textures/skybox/indoor.hdr"));
	}

	SkyboxManager::~SkyboxManager() {
		for(auto& [name, skybox] : m_Skyboxes) {
			DELETE_PTR(skybox);
		}
	}

	PreethamSky* SkyboxManager::getPreethamSky() const {
		return (PreethamSky*)m_Skyboxes.at(PREETHAM_SKYBOX_NAME);
	}

	Skybox* SkyboxManager::getIndoorSkybox() const {
		return m_Skyboxes.at(INDOOR_SKYBOX_NAME);
	}

	Skybox* SkyboxManager::load(const String& name, const String& filename) {

		if(exists(name)) return m_Skyboxes[name];

		String extension = Files::extension(filename);

		Skybox* skybox = nullptr;

		if(extension == ".skybox") {
			// TODO: load from skybox file
		} else {
			// Load from equirectangular image
			skybox = m_SkyboxFactory->create(name, filename);
		}

		m_Skyboxes[name] = skybox;

		return skybox;
	}

	bool SkyboxManager::exists(const String& name) const {
		return m_Skyboxes.find(name) != m_Skyboxes.end();
	}

	Skybox* SkyboxManager::find(const String& name) const {
		return exists(name) ? m_Skyboxes.at(name) : nullptr;
	}

	void SkyboxManager::destroy(const String& name) {
		if(!exists(name)) return;
		Skybox* skybox = m_Skyboxes[name];
		DELETE_PTR(skybox);
		m_Skyboxes.erase(name);
	}

	void SkyboxManager::updatePreethamSky(PreethamSky* sky) {
		if(sky == nullptr) return;
		m_SkyboxFactory->updatePreethamSky(sky);
	}

	void SkyboxManager::createPreethamSky() {
		m_Skyboxes[PREETHAM_SKYBOX_NAME] = m_SkyboxFactory->createPreethamSky(PREETHAM_SKYBOX_NAME, SkyboxLoadInfo(), 2, 0, 0);
		Log::debug("Created Preetham sky");
	}

	Skybox* SkyboxManager::createSkybox(const String& name, const String& filename) {
		return nullptr;
	}

	Skybox* SkyboxManager::loadCache(const String& originalFilename, const String& cacheFilename) {
		// TODO
		return nullptr;
	}

	void SkyboxManager::loadCachedSkyboxes() {
		// TODO
	}

	void SkyboxManager::createSkyboxCache(const String& filename, Skybox* skybox) {
		// TODO
	}

	String SkyboxManager::getFilenameFromCache(const String& cacheFilename) {
		return milo::String();
	}

	String SkyboxManager::getNameFromFilename(const String& cacheFilename) {
		return milo::String();
	}
}
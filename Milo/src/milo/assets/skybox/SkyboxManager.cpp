#include "milo/assets/skybox/SkyboxManager.h"
#include "milo/io/Files.h"

namespace milo {

	SkyboxManager::SkyboxManager() {
		load(DEFAULT_SKYBOX_NAME, Files::resource("skybox/...")); // TODO
	}

	SkyboxManager::~SkyboxManager() {
		for(auto& [name, skybox] : m_Skyboxes) {
			DELETE_PTR(skybox);
		}
	}

	Skybox* SkyboxManager::getDefault() const {
		return m_Skyboxes.at(DEFAULT_SKYBOX_NAME);
	}

	Skybox* SkyboxManager::load(const String& name, const String& filename) {

		if(exists(name)) return m_Skyboxes[name];

		String extension = Files::extension(filename);

		Skybox* skybox;

		if(extension == ".skybox") {
			// TODO: load from skybox file
		} else {
			// Load from equirectangular image

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
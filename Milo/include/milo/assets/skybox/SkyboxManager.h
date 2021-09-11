#pragma once

#include "SkyboxFactory.h"

namespace milo {

	class SkyboxManager {
		friend class AssetManager;
	private:
		HashMap<String, Skybox*> m_Skyboxes;
		SkyboxFactory* m_SkyboxFactory{nullptr};
	private:
		SkyboxManager();
		~SkyboxManager();
	public:
		PreethamSky* getPreethamSky() const;
		Skybox* getIndoorSkybox() const;
		Skybox* load(const String& name, const String& filename);
		bool exists(const String& name) const;
		Skybox* find(const String& name) const;
		void destroy(const String& name);
		void updatePreethamSky(PreethamSky* sky);
	private:
		void createPreethamSky();
		Skybox* createSkybox(const String& name, const String& filename);
		Skybox* loadCache(const String& originalFilename, const String& cacheFilename);
		void loadCachedSkyboxes();
		void createSkyboxCache(const String& filename, Skybox* shader);
		static String getFilenameFromCache(const String& cacheFilename);
		static String getNameFromFilename(const String& cacheFilename);
	};

}
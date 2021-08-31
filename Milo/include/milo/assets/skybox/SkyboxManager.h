#pragma once

#include "SkyboxFactory.h"

namespace milo {

	const String DEFAULT_SKYBOX_NAME = "SK_Default";

	class SkyboxManager {
		friend class AssetManager;
	private:
		HashMap<String, Skybox*> m_Skyboxes;
		SkyboxFactory* m_SkyboxFactory{nullptr};
	private:
		SkyboxManager();
		~SkyboxManager();
	public:
		Skybox* getDefault() const;
		Skybox* load(const String& name, const String& filename);
		bool exists(const String& name) const;
		Skybox* find(const String& name) const;
		void destroy(const String& name);
	private:
		Skybox* createSkybox(const String& name, const String& filename);
		Skybox* loadCache(const String& originalFilename, const String& cacheFilename);
		void loadCachedSkyboxes();
		void createSkyboxCache(const String& filename, Skybox* shader);
		static String getFilenameFromCache(const String& cacheFilename);
		static String getNameFromFilename(const String& cacheFilename);
	};

}
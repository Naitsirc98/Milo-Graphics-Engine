#pragma once

#include "Material.h"
#include "MaterialResourcePool.h"

namespace milo {

	class MaterialManager {
		friend class AssetManager;
	private:
		HashMap<String, Material*> m_Materials;
		Mutex m_Mutex;
		Texture2D* m_WhiteTexture{nullptr};
		Texture2D* m_BlackTexture{nullptr};
		MaterialResourcePool* m_ResourcePool{nullptr};
	private:
		MaterialManager();
		~MaterialManager();
	public:
		Material* getDefault() const;
		Material* load(const String& name, const String& filename);
		bool exists(const String& name);
		Material* find(const String& name);
		void destroy(const String& name);
		MaterialResourcePool& resourcePool() const;
	private:
		bool load(const String& name, const String& filename, Material*& material);
		Texture2D* loadTexture2D(void* pJson, const String& textureName, const String& materialFile);
	private:
		static Texture2D* createWhiteTexture();
		static Texture2D* createBlackTexture();
	};

}
#pragma once

#include "Material.h"
#include "MaterialResourcePool.h"

namespace milo {

	class MaterialManager {
		friend class AssetManager;
		friend class AssimpModelLoader;
		friend class MiloEngine;
	private:
		HashMap<String, Material*> m_Materials;
		Mutex m_Mutex;
		MaterialResourcePool* m_ResourcePool{nullptr};
	private:
		MaterialManager();
		~MaterialManager();
		void init();
	public:
		Material* getDefault() const;
		Material* create(const String& name, const String& filename = "");
		Material* load(const String& name, const String& filename);
		bool exists(const String& name);
		Material* find(const String& name);
		void destroy(const String& name);
		MaterialResourcePool& resourcePool() const;
	private:
		void addMaterial(const String& name, Material* material);
		bool load(const String& name, const String& filename, Material*& material);
		Ref<Texture2D> loadTexture2D(void* pJson, const String& textureName, const String& materialFile);
		void update();
	};

}
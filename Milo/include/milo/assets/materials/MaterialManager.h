#pragma once

#include "Material.h"

namespace milo {

	class MaterialManager {
		friend class AssetManager;
	private:
		HashMap<String, Material*> m_Materials;
		Mutex m_Mutex;
	private:
		MaterialManager();
		~MaterialManager();
	public:
		Material* getDefault() const;
		Material* load(const String& name, const String& filename);
		bool exists(const String& name);
		Material* find(const String& name);
		void destroy(const String& name);
	private:
		bool load(const String& name, const String& filename, Material*& material);
	};

}
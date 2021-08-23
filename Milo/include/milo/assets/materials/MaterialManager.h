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
		Material* load(const String& filename);
		bool exists(const String& filename);
		Material* find(const String& filename);
		void destroy(const String& filename);
	private:
		bool load(const String& filename, Material*& material);
	};

}
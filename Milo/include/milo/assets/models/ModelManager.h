#pragma once

#include "ModelLoader.h"

namespace milo {

	class ModelManager {
		friend class AssetManager;
	private:
		HashMap<String, Model*> m_Models;
	private:
		ModelManager();
		virtual ~ModelManager();
		void init();
	public:
	public:
		Model* load(const String& name, const String& filename);
		bool exists(const String& name) const;
		Model* find(const String& name) const;
		void destroy(const String& name);
	};

}
#include "milo/assets/models/ModelManager.h"
#include "milo/assets/AssetManager.h"
#include "milo/assets/models/loaders/AssimpModelLoader.h"

namespace milo {

	ModelManager::ModelManager() {

	}

	ModelManager::~ModelManager() {
		for(auto& [name, model] : m_Models) {
			DELETE_PTR(model);
		}
	}

	void ModelManager::init() {
		load("Sponza", "resources/models/Sponza/Sponza.gltf");
	}

	Model* ModelManager::load(const String& name, const String& filename) {
		if(exists(name)) return m_Models[name];
		Log::debug("Loading model {}...", name);
		float start = Time::millis();
		Model* model = AssimpModelLoader().load(filename);
		Log::debug("Model {} loaded in {} ms", name, Time::millis() - start);
		if(model != nullptr) {
			model->m_Name = name;
			model->m_Filename = filename;
			// model->m_Icon = Assets::textures().createIcon(name, model); TODO
			m_Models[name] = model;
		}
		return model;
	}

	bool ModelManager::exists(const String& name) const {
		return m_Models.find(name) != m_Models.end();
	}

	Model* ModelManager::find(const String& name) const {
		return exists(name) ? m_Models.at(name) : nullptr;
	}

	void ModelManager::destroy(const String& name) {
		if(!exists(name)) return;

		Model* model = m_Models[name];
		m_Models.erase(name);

		DELETE_PTR(model);
	}

}

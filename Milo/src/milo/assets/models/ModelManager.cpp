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
		// TODO
		//Model* sponza = load("Sponza", "resources/models/Sponza/Sponza.gltf");
		//sponza->m_Nodes[0]->transform = scale(Matrix4(1.0), {0.1f, 0.1f, 0.1f});
		//sponza->setCanBeCulled(false);
		Model* damagedHelmet = load("DamagedHelmet", "resources/models/DamagedHelmet/DamagedHelmet.gltf");
		damagedHelmet->setCanBeCulled(true);
	}

	Model* ModelManager::getSponza() const {
		return m_Models.at("Sponza");
	}

	Model* ModelManager::getDamagedHelmet() const {
		return m_Models.at("DamagedHelmet");
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

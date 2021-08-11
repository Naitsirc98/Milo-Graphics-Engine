#include "milo/scenes/Scene.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"

namespace milo {

	Scene::Scene(const String& name) : m_Name(name) {
	}

	Scene::Scene(String&& name) : m_Name(std::move(name)) {
	}

	Scene::~Scene() = default;

	const String &Scene::name() const noexcept {
		return m_Name;
	}

	bool Scene::isActiveScene() const {
		return SceneManager::activeScene() == this;
	}

	Entity Scene::createEntity() {
		const EntityId newId = m_Registry.create();
		return {newId, *this};
	}

	bool Scene::exists(EntityId entityId) const {
		return m_Registry.valid(entityId);
	}

	Optional<Entity> Scene::find(EntityId id) const {
		if(exists(id)) return Entity(id, const_cast<Scene &>(*this));
		return {};
	}

	void Scene::destroyEntity(EntityId entityId) noexcept {
		if(!exists(entityId)) return;
		m_Registry.destroy(entityId);
	}

	ECSRegistry &Scene::registry() noexcept {
		return m_Registry;
	}

	void Scene::update() {

	}

	void Scene::lateUpdate() {

	}

	void Scene::render() {

	}

	void Scene::renderUI() {

	}
}

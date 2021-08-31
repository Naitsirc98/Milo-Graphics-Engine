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

	Entity Scene::find(EntityId id) const {
		if(!exists(id)) throw MILO_RUNTIME_EXCEPTION(str("Entity ") + str((uint64_t)id) + " does not exists");
		return Entity(id, const_cast<Scene&>(*this));
	}

	void Scene::destroyEntity(EntityId entityId) noexcept {
		if(!exists(entityId)) return;
		m_Registry.destroy(entityId);
	}

	Entity Scene::cameraEntity() noexcept {
		return Entity(m_MainCameraEntity, *this);
	}

	Camera* Scene::camera() noexcept {
		Entity entity = cameraEntity();
		if(entity.valid()) return &entity.getComponent<Camera>();
		return nullptr;
	}

	void Scene::setMainCamera(EntityId id) noexcept {
		m_MainCameraEntity = id;
	}

	ECSRegistry& Scene::registry() noexcept {
		return m_Registry;
	}

	Skybox* Scene::skybox() const {
		return m_Skybox;
	}

	void Scene::setSkybox(Skybox* skybox) {
		m_Skybox = skybox;
	}

	void Scene::update() {
		auto nativeScripts = m_Registry.view<NativeScriptView>();
		for(EntityId entity : nativeScripts) {
			auto& nativeScriptView = m_Registry.get<NativeScriptView>(entity);
			nativeScriptView.createIfNotExists(entity);
			nativeScriptView.script->onUpdate(entity);
		}
	}

	void Scene::lateUpdate() {
		const ECSComponentView<NativeScriptView> nativeScripts = m_Registry.view<NativeScriptView>();
		for(EntityId entity : nativeScripts) {
			auto& nativeScriptView = m_Registry.get<NativeScriptView>(entity);
			nativeScriptView.createIfNotExists(entity);
			nativeScriptView.script->onLateUpdate(entity);
		}
	}

	void Scene::render() {

	}

	void Scene::renderUI() {

	}
}
